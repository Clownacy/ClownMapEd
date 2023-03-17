#include "tile-viewer.h"

#include <cstddef>

#include <QIcon>

#include "pixmap-button.h"

TileViewer::TileViewer(const TilePixmaps &tile_pixmaps)
	: tile_pixmaps(tile_pixmaps)
	, flow_layout(0, 0, 0)
{
	setLayout(&flow_layout);

	refresh();
}

void TileViewer::refresh()
{
	for (QLayoutItem *item = flow_layout.itemAt(0); item != nullptr; item = flow_layout.itemAt(0))
		flow_layout.removeItem(item);

	for (std::size_t i = 0; i < tile_pixmaps.total_pixmaps(); ++i)
	{
		PixmapButton* const button = new PixmapButton(this);

		button->setPixmap(tile_pixmaps[i]);

		button->setFixedSize(24, 24);

		flow_layout.addWidget(button);
	}
}

void TileViewer::setBackgroundColour(const QColor &colour)
{
	QPalette palette;
	palette.setColor(QPalette::Window, colour);
	setPalette(palette);
}
