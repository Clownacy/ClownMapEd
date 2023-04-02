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

	connect(&tile_manager, &TileManager::regenerated, this,
		[this]()
		{
			selected.resize(this->tile_manager.total_tiles());
			update();
		}
	);

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

void TileViewer::setSelection(const std::function<void(QVector<bool> &selection)> &callback)
{
	selected.fill(false);
	callback(selected);

	int width, height;
	getGridDimensions(width, height);

	const int row_offset = scroll() % width;

	const auto to_row = [&width, &row_offset](const int tile_index)
	{
		return (tile_index - row_offset) / width;
	};

	const int scroll_row = to_row(scroll());

	const int first_tile_row = to_row(selected.indexOf(true));
	const int last_tile_row = to_row(selected.lastIndexOf(true));

	if (first_tile_row < scroll_row + 2 || last_tile_row > scroll_row + height - 1 - 2)
	{
		int y;

		if (first_tile_row < scroll_row + 2)
			y = first_tile_row - 2;
		else //if (last_tile_row > scroll_row + height - 1 - 2)
			y = last_tile_row - height + 1 + 2;

		setScroll(row_offset + y * width);
	}

	update();
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

	QBrush brush;
	brush.setStyle(Qt::SolidPattern);

	QPainter painter(this);
	painter.setPen(Qt::NoPen);

	const int tiles_to_do = tile_manager.total_tiles() - m_scroll;
	const int rows_to_do = qMin(total_rows, DivideCeiling(tiles_to_do, tiles_per_row));

	for (int y = 0; y < rows_to_do; ++y)
	{
		const int length_of_row = qMin(tiles_per_row, tiles_to_do - y * tiles_per_row);

		// Draw background colour for this row.
		brush.setColor(background_colour);
		painter.setBrush(brush);
		painter.drawRect(0, y * TILE_HEIGHT_SCALED, length_of_row * TILE_WIDTH_SCALED, TILE_HEIGHT_SCALED);

		// Draw tiles in this row.
		brush.setColor(Qt::white);
		painter.setBrush(brush);
		for (int x = 0; x < length_of_row; ++x)
		{
			const QRect rect(x * TILE_WIDTH_SCALED, y * TILE_HEIGHT_SCALED, TILE_WIDTH_SCALED, TILE_HEIGHT_SCALED);

			// Draw the tile.
			painter.drawPixmap(rect, tile_manager.pixmaps(current_tile++, palette_line));

			if (selection_flip_flop && selected[current_tile])
			{
				// Invert the tile if selected.
				painter.setCompositionMode(QPainter::RasterOp_SourceXorDestination);
				painter.drawRect(rect);
				painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
			}
		}
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
	{
		const int tile_index = tile_y * tiles_per_row + tile_x;

		setSelection(
			[tile_index](QVector<bool> &selection)
			{
				selection[tile_index];
			}
		);

		emit tileSelected(tile_index);
	}
}
