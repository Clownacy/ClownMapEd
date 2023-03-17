#include "tile-viewer.h"

#include <cstddef>

#include <QtGlobal>
#include <QPainter>

TileViewer::TileViewer(const TileManager &tile_manager)
	: tile_manager(tile_manager)
{
	this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	refresh();
}

void TileViewer::refresh()
{

}

void TileViewer::setBackgroundColour(const QColor &colour)
{
	background_colour = colour;
}

template <typename T>
static inline T DivideCeiling(T a, T b)
{
	return (a + (b - 1)) / b;
}

void TileViewer::paintEvent(QPaintEvent* const event)
{
	const std::size_t size_of_tile = 24;
	const std::size_t tiles_per_row = width() / size_of_tile;
	const std::size_t total_rows = qMin(height() / size_of_tile, DivideCeiling(tile_manager.total_tiles(), tiles_per_row));

	std::size_t current_tile = 0;

	QPainter painter(this);

	QBrush brush;
	brush.setColor(background_colour);
	brush.setStyle(Qt::SolidPattern);
	painter.setBrush(brush);
	painter.setPen(Qt::NoPen);

	for (std::size_t y = 0; y < total_rows; ++y)
	{
		const std::size_t length_of_row = qMin(tiles_per_row, tile_manager.total_tiles() - y * tiles_per_row);
		painter.drawRect(QRect(0, y * size_of_tile, length_of_row * size_of_tile, size_of_tile));

		for (std::size_t x = 0; x < length_of_row; ++x)
			painter.drawPixmap(QRect(x * size_of_tile, y * size_of_tile, size_of_tile, size_of_tile), tile_manager.pixmaps(current_tile++));
	}
}
