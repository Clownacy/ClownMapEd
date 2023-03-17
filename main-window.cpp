#include "main-window.h"
#include "./ui_main-window.h"

#include <QFileDialog>
#include <QKeyEvent>

MainWindow::MainWindow(QWidget* const parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
    , tile_manager(&tiles_bytes, &palette)
	, sprite_viewer(tile_manager, sprite_mappings)
    , palette_editor(palette)
	, piece_picker(tile_manager)
    , tile_viewer(tile_manager)
{
	ui->setupUi(this);

	horizontal_box.addWidget(&piece_picker);
	horizontal_box.addWidget(&palette_editor);
	horizontal_box.addWidget(&sprite_viewer);

	vertical_box.addLayout(&horizontal_box);
//	vbox->addsp
	vertical_box.addWidget(&tile_viewer);

	centralWidget()->setLayout(&vertical_box);

	sprite_viewer.setBackgroundColour(palette.getColourQColor(0, 0));
	tile_viewer.setBackgroundColour(palette.getColourQColor(0, 0));

	horizontal_box.setMargin(vertical_box.margin());
	vertical_box.setMargin(0);

	connect(ui->actionOpen_Tiles, &QAction::triggered, this,
		[this]()
		{
			const QString file_path = QFileDialog::getOpenFileName(this, "Open Tile File");

			if (!file_path.isNull())
			{
				QFile file(file_path);
				if (!file.open(QFile::ReadOnly))
					return;

				QDataStream stream(&file);

				tiles_bytes.resize(file.size());
				tiles_bytes.squeeze();

				stream.readRawData(reinterpret_cast<char*>(tiles_bytes.data()), tiles_bytes.size());

				tile_manager.setTiles(tiles_bytes);
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
			}
		}
	);

	connect(ui->actionOpen_Mappings, &QAction::triggered, this,
		[this]()
		{
			const QString file_path = QFileDialog::getOpenFileName(this, "Open Mappings File");

			if (!file_path.isNull())
			{
				QFile file(file_path);
				if (!file.open(QFile::ReadOnly))
					return;

				sprite_mappings = SpriteMappings::fromFile(file);
			}
		}
	);

	connect(&palette, &Palette::singleColourChanged, &tile_manager, &TileManager::regenerate);

	connect(&palette, &Palette::singleColourChanged, this,
		[this](const unsigned int palette_line, const unsigned int palette_index, const QColor &colour)
		{
			if (palette_line == 0 && palette_index == 0)
			{
				sprite_viewer.setBackgroundColour(colour);
				tile_viewer.setBackgroundColour(colour);
			}
		}
	);

	connect(&palette, &Palette::allColoursChanged, this,
		[this]()
		{
			sprite_viewer.setBackgroundColour(palette.getColourQColor(0, 0));
			tile_viewer.setBackgroundColour(palette.getColourQColor(0, 0));
		}
	);

	connect(&tile_viewer, &TileViewer::tileSelected, &piece_picker, &SpritePiecePicker::setSelectedTile);

	connect(&tile_manager, &TileManager::regenerated, &tile_viewer,
		[this]()
		{
			piece_picker.update();
			sprite_viewer.update();
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
			sprite_viewer.selectPreviousSprite();
			break;

		case Qt::Key::Key_BracketRight:
			sprite_viewer.selectNextSprite();
			break;
	}
}
