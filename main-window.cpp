#include "main-window.h"
#include "./ui_main-window.h"

#include <fstream>

#include <QDataStream>
#include <QFile>
#include <QFileDialog>
#include <QKeyEvent>

#include "kosinski.h"
#include "nemesis.h"

MainWindow::MainWindow(QWidget* const parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
    , tile_manager(nullptr, 0, palette)
	, sprite_editor(tile_manager, sprite_mappings)
    , palette_editor(palette)
	, sprite_piece_picker(tile_manager)
    , tile_viewer(tile_manager)
{
	ui->setupUi(this);

	horizontal_layout.addWidget(&sprite_piece_picker);
	horizontal_layout.addWidget(&palette_editor);
	horizontal_layout.addWidget(&sprite_editor);

	vertical_layout.addLayout(&horizontal_layout);
//	vbox->addsp
	vertical_layout.addWidget(&tile_viewer);

	centralWidget()->setLayout(&vertical_layout);

	sprite_editor.setBackgroundColour(palette.getColourQColor(0, 0));
	tile_viewer.setBackgroundColour(palette.getColourQColor(0, 0));

	horizontal_layout.setMargin(vertical_layout.margin());
	vertical_layout.setMargin(0);

	connect(ui->actionOpen_Tiles, &QAction::triggered, this,
		[this]()
		{
			const QString file_path = QFileDialog::getOpenFileName(this, "Open Tile Graphics File");

			if (!file_path.isNull())
			{
				QFile file(file_path);

				if (!file.open(QFile::ReadOnly))
					return;

				QDataStream stream(&file);

				QVector<uchar> bytes;
				bytes.resize(file.size());
				stream.readRawData(reinterpret_cast<char*>(bytes.data()), bytes.size());

				if (!tile_manager.setTiles(bytes.data(), bytes.size()))
				{
					// TODO: Should probably show an error message or something.
				}
			}
		}
	);

	connect(ui->actionOpen_Nemesis_Compressed_Graphics, &QAction::triggered, this,
		[this]()
		{
			const QString file_path = QFileDialog::getOpenFileName(this, "Open Tile Graphics File");

			if (!file_path.isNull())
			{
				std::ifstream file_stream(file_path.toStdString(), std::ifstream::in | std::ifstream::binary);

				if (!file_stream.is_open())
					return;

				std::stringstream string_stream(std::stringstream::in | std::stringstream::out | std::stringstream::binary);

				if (!nemesis::decode(file_stream, string_stream))
					return;

				if (!tile_manager.setTiles(reinterpret_cast<const uchar*>(string_stream.str().c_str()), static_cast<int>(string_stream.str().length())))
				{
					// TODO: Should probably show an error message or something.
				}
			}
		}
	);

	connect(ui->actionLoad_Kosinski_Compressed_Tile_Graphics, &QAction::triggered, this,
		[this]()
		{
			const QString file_path = QFileDialog::getOpenFileName(this, "Open Tile Graphics File");

			if (!file_path.isNull())
			{
				std::ifstream file_stream(file_path.toStdString(), std::ifstream::in | std::ifstream::binary);

				if (!file_stream.is_open())
					return;

				std::stringstream string_stream(std::stringstream::in | std::stringstream::out | std::stringstream::binary);

				if (!kosinski::decode(file_stream, string_stream))
					return;

				if (!tile_manager.setTiles(reinterpret_cast<const uchar*>(string_stream.str().c_str()), static_cast<int>(string_stream.str().length())))
				{
					// TODO: Should probably show an error message or something.
				}
			}
		}
	);

	connect(ui->actionLoad_KosinskiM_Compressed_Tile_Graphics, &QAction::triggered, this,
		[this]()
		{
			const QString file_path = QFileDialog::getOpenFileName(this, "Open Tile Graphics File");

			if (!file_path.isNull())
			{
				std::ifstream file_stream(file_path.toStdString(), std::ifstream::in | std::ifstream::binary);

				if (!file_stream.is_open())
					return;

				std::stringstream string_stream(std::stringstream::in | std::stringstream::out | std::stringstream::binary);

				if (!kosinski::moduled_decode(file_stream, string_stream))
					return;

				if (!tile_manager.setTiles(reinterpret_cast<const uchar*>(string_stream.str().c_str()), static_cast<int>(string_stream.str().length())))
				{
					// TODO: Should probably show an error message or something.
				}
			}
		}
	);

	connect(ui->actionOpen, &QAction::triggered, this,
		[this]()
		{
			const QString file_path = QFileDialog::getOpenFileName(this, "Open Palette File");

			if (!file_path.isNull())
			{
				palette.loadFromFile(file_path);
				tile_manager.setPalette(palette);
				palette_editor.update();
			}
		}
	);

	connect(ui->actionOpen_Mappings, &QAction::triggered, this,
		[this]()
		{
			const QString file_path = QFileDialog::getOpenFileName(this, "Open Sprite Mappings File");

			if (!file_path.isNull())
			{
				QFile file(file_path);
				if (!file.open(QFile::ReadOnly))
					return;

				sprite_mappings = SpriteMappings::fromFile(file);
				sprite_editor.update();
			}
		}
	);

	connect(&palette, &Palette::singleColourChanged, &tile_manager, &TileManager::regenerate);

	connect(&palette, &Palette::singleColourChanged, this,
		[this](const unsigned int palette_line, const unsigned int palette_index, const QColor &colour)
		{
			if (palette_line == 0 && palette_index == 0)
			{
				sprite_editor.setBackgroundColour(colour);
				tile_viewer.setBackgroundColour(colour);
				sprite_piece_picker.setBackgroundColour(colour);
			}
		}
	);

	connect(&palette, &Palette::allColoursChanged, this,
		[this]()
		{
			const QColor &background_colour = palette.getColourQColor(0, 0);

			sprite_editor.setBackgroundColour(background_colour);
			tile_viewer.setBackgroundColour(background_colour);
			sprite_piece_picker.setBackgroundColour(background_colour);
		}
	);

	connect(&tile_viewer, &TileViewer::tileSelected, &sprite_piece_picker, &SpritePiecePicker::setSelectedTile);

	connect(&tile_manager, &TileManager::regenerated, &tile_viewer,
		[this]()
		{
			sprite_piece_picker.update();
			sprite_editor.update();
			tile_viewer.update();
		}
	);
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::keyPressEvent(QKeyEvent* const event)
{
	switch (event->key())
	{
		case Qt::Key::Key_BracketLeft:
			sprite_editor.selectPreviousSprite();
			break;

		case Qt::Key::Key_BracketRight:
			sprite_editor.selectNextSprite();
			break;

		case Qt::Key::Key_1:
		case Qt::Key::Key_2:
		case Qt::Key::Key_3:
		case Qt::Key::Key_4:
		{
			const int palette_line = event->key() - Qt::Key::Key_1;
			sprite_piece_picker.setPaletteLine(palette_line);
			sprite_editor.setStartingPaletteLine(palette_line);
			tile_viewer.setPaletteLine(palette_line);
			break;
		}
	}
}
