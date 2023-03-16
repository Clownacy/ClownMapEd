#include "main-window.h"
#include "./ui_main-window.h"

#include <QColorDialog>
#include <QFileDialog>
#include <QGraphicsPixmapItem>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QPushButton>

#include "colour-button.h"

MainWindow::MainWindow(QWidget* const parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
    , palette()
    , tiles_bytes()
    , sprite_mappings()
    , tile_pixmaps(this->tiles_bytes, this->palette)
	, sprite_viewer(this->tile_pixmaps, this->sprite_mappings)
{
	ui->setupUi(this);

	connect(ui->actionOpen_Tiles, &QAction::triggered, this,
		[this]()
		{
			const QString file_path = QFileDialog::getOpenFileName(this, "Open Tile File");

			if (!file_path.isNull())
			{
				QFile file(file_path);
				if (!file.open(QFile::ReadOnly))
					return;

				QDataStream in_stream(&file);

				this->tiles_bytes.resize(file.size());

				in_stream.readRawData(reinterpret_cast<char*>(this->tiles_bytes.data()), this->tiles_bytes.size());

				this->tile_pixmaps = TilePixmaps(this->tiles_bytes, this->palette);
			}
		}
	);

	connect(ui->actionOpen, &QAction::triggered, this,
		[this]()
		{
			const QString file_path = QFileDialog::getOpenFileName(this, "Open Palette File");

			if (!file_path.isNull())
			{
				this->palette.loadFromFile(file_path);
				this->tile_pixmaps = TilePixmaps(this->tiles_bytes, this->palette);

				const Palette::Colour colour = this->palette.getColour(0, 0);
				sprite_viewer.setBackgroundColour(QColor(colour.red, colour.green, colour.blue));
			}
		}
	);

	connect(ui->actionOpen_Mappings, &QAction::triggered, this,
		[this]()
		{
			const QString file_path = QFileDialog::getOpenFileName(this, "Open Mappings File");

			if (!file_path.isNull())
				this->sprite_mappings.loadFromFile(file_path);
		}
	);

	QGridLayout *grid_layout = new QGridLayout;
	grid_layout->setSpacing(0);

	for (unsigned int line = 0; line < 4; ++line)
	{
		for (unsigned int index = 0; index < 16; ++index)
		{
			ColourButton *button = new ColourButton();

			connect(button, &ColourButton::clicked, this,
				[this]()
				{
					QColorDialog::getColor(Qt::white, this);
				}
			);

			button->setColour(palette.getColourQColor(line, index));

			button->setFixedSize(20, 20);
	//		button->setIcon(QIcon(QPixmap()))

			grid_layout->addWidget(button, index, line);
		}
	}

	QHBoxLayout *hbox = new QHBoxLayout;
	hbox->addLayout(grid_layout);
	hbox->addWidget(&sprite_viewer);

	QVBoxLayout *vbox = new QVBoxLayout;
	vbox->addLayout(hbox);
//	vbox->addsp
	vbox->addWidget(new QLabel("HOW"));

	this->centralWidget()->setLayout(vbox);

	const Palette::Colour colour = this->palette.getColour(0, 0);
	sprite_viewer.setBackgroundColour(QColor(colour.red, colour.green, colour.blue));
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
			this->sprite_viewer.selectPreviousSprite();
			break;

		case Qt::Key::Key_BracketRight:
			this->sprite_viewer.selectNextSprite();
			break;
	}
}
