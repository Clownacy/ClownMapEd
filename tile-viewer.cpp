#include "tile-viewer.h"

#include <cstddef>

#include <QtGlobal>
#include <QMouseEvent>
#include <QPainter>

static constexpr int TILE_WIDTH_SCALED = Tile::WIDTH * 3;
static constexpr int TILE_HEIGHT_SCALED = Tile::HEIGHT * 3;

template <typename T>
static inline T DivideCeiling(T a, T b)
{
	return (a + (b - 1)) / b;
}

TileViewer::TileViewer(const TileManager &tile_manager)
	: tile_manager(tile_manager)
	, timer(this)
{
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	connect(&tile_manager, &TileManager::regenerated, this, qOverload<>(&TileViewer::update));

	connect(&timer, &QTimer::timeout, this,
		[this]()
		{
			selection_flip_flop = !selection_flip_flop;
			update();
		}
	);

	// Flash the selection every two seconds.
	timer.start(1000 * 2);
}

void TileViewer::setSelection(const int start, const int end)
{
	selection_start = start;
	selection_end = end;
	update();

	emit tileSelected(selection_start);
}

void TileViewer::getGridDimensions(int &columns, int &rows)
{
	columns = width() / TILE_WIDTH_SCALED;
	rows = qMin(DivideCeiling(height(), TILE_HEIGHT_SCALED), DivideCeiling(tile_manager.total_tiles(), columns));
}

void TileViewer::paintEvent(QPaintEvent* const event)
{
	QWidget::paintEvent(event);

	int tiles_per_row, total_rows;
	getGridDimensions(tiles_per_row, total_rows);

	int current_tile = m_scroll;

	QPainter painter(this);

	QBrush brush;
	brush.setColor(background_colour);
	brush.setStyle(Qt::SolidPattern);
	painter.setBrush(brush);
	painter.setPen(Qt::NoPen);

	const int tiles_to_do = tile_manager.total_tiles() - m_scroll;
	const int rows_to_do = qMin(total_rows, DivideCeiling(tiles_to_do, tiles_per_row));

	for (int y = 0; y < rows_to_do; ++y)
	{
		const int length_of_row = qMin(tiles_per_row, tiles_to_do - y * tiles_per_row);

		// Draw background colour for this row.
		painter.drawRect(0, y * TILE_HEIGHT_SCALED, length_of_row * TILE_WIDTH_SCALED, TILE_HEIGHT_SCALED);

		// Draw tiles in this row.
		for (int x = 0; x < length_of_row; ++x)
			painter.drawPixmap(x * TILE_WIDTH_SCALED, y * TILE_HEIGHT_SCALED, TILE_WIDTH_SCALED, TILE_HEIGHT_SCALED, tile_manager.pixmaps(current_tile++, palette_line));
	}

	// Draw the selection.
	if (selection_flip_flop)
	{
		brush.setColor(Qt::white);
		painter.setBrush(brush);
		painter.setCompositionMode(QPainter::RasterOp_SourceXorDestination);

		for (int i = selection_start; i < selection_end; ++i)
			painter.drawRect((i % tiles_per_row) * TILE_WIDTH_SCALED, (i / tiles_per_row) * TILE_HEIGHT_SCALED, TILE_WIDTH_SCALED, TILE_HEIGHT_SCALED);
	}
}

void TileViewer::mousePressEvent(QMouseEvent* const event)
{
	QWidget::mousePressEvent(event);

	int tiles_per_row, total_rows;
	getGridDimensions(tiles_per_row, total_rows);

	const int tile_x = event->x() / TILE_WIDTH_SCALED;
	const int tile_y = event->y() / TILE_HEIGHT_SCALED;

	if (tile_x < tiles_per_row && tile_y < total_rows)
		setSelection(tile_y * tiles_per_row + tile_x, selection_start + 1);
}
