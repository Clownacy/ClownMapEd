#include "tile-viewer.h"

#include <QtAlgorithms>
#include <QtGlobal>
#include <QMouseEvent>
#include <QPainter>

#include "utilities.h"

static constexpr int TILE_WIDTH_SCALED = TileManager::TILE_WIDTH * 2;
static constexpr int TILE_HEIGHT_SCALED = TileManager::TILE_HEIGHT * 2;

TileViewer::TileViewer(const TileManager &tile_manager)
	: tile_manager(tile_manager)
	, timer(this)
{
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	connect(&tile_manager, &TileManager::pixmapsChanged, this,
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

void TileViewer::setSelection(const bool scroll_to_selection, const std::function<void(QVector<bool> &selection)> &callback)
{
	callback(selected);

	if (scroll_to_selection)
	{
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

		const int upper_bound_delta = 2;
		const int lower_bound_delta = height - 1 - 2;
		const int upper_bound = scroll_row + upper_bound_delta;
		const int lower_bound = scroll_row + lower_bound_delta;

		if (first_tile_row < upper_bound || last_tile_row > lower_bound)
		{
			int y;

			if (last_tile_row > lower_bound)
				y = qMin(first_tile_row - upper_bound_delta, last_tile_row - lower_bound_delta);
			else //if (first_tile_row < upper_bound)
				y = qMax(first_tile_row - upper_bound_delta, last_tile_row - lower_bound_delta);

			setScroll(row_offset + y * width);
		}
	}

	update();
}

void TileViewer::getGridDimensions(int &columns, int &rows)
{
	columns = width() / TILE_WIDTH_SCALED;
	rows = qMax(1, qMin(Utilities::DivideCeiling(height(), TILE_HEIGHT_SCALED), Utilities::DivideCeiling(tile_manager.total_tiles(), columns)));

	if (vertical_orientation)
		qSwap(columns, rows);
}

void TileViewer::paintEvent(QPaintEvent* const event)
{
	QWidget::paintEvent(event);

	int tiles_per_row, total_rows;
	getGridDimensions(tiles_per_row, total_rows);

	QBrush brush;
	brush.setStyle(Qt::SolidPattern);

	QPainter painter(this);
	painter.setPen(Qt::NoPen);

	const int tiles_to_do = tile_manager.total_tiles() - m_scroll;
	const int rows_to_do = qMin(total_rows, Utilities::DivideCeiling(tiles_to_do, tiles_per_row));

	const auto do_visible_rows = [this, tiles_per_row, tiles_to_do, rows_to_do](const std::function<void(int, int, int)> &callback)
	{
		int current_tile = m_scroll;

		for (int y = 0; y < rows_to_do; ++y)
		{
			const int length_of_row = qMin(tiles_per_row, tiles_to_do - y * tiles_per_row);

			callback(current_tile, y, length_of_row);

			current_tile += length_of_row;
		}
	};

	const auto do_visible_tiles = [this, &do_visible_rows](const std::function<void(int, const QRect&)> &callback)
	{
		do_visible_rows([this, &callback](const int tile_index, const int y, const int length_of_row)
			{
				for (int x = 0; x < length_of_row; ++x)
				{
					const int actual_x = vertical_orientation ? y : x;
					const int actual_y = vertical_orientation ? x : y;
					callback(tile_index + x, QRect(actual_x * TILE_WIDTH_SCALED, actual_y * TILE_HEIGHT_SCALED, TILE_WIDTH_SCALED, TILE_HEIGHT_SCALED));
				}
			}
		);
	};

	brush.setColor(Qt::black);
	painter.setBrush(brush);

	// Initialise colour values to 0.
	do_visible_rows(
		[this, &painter](const int tile_index, const int y, const int length_of_row)
		{
			Q_UNUSED(tile_index);

			if (vertical_orientation)
				painter.drawRect(QRect{y * TILE_WIDTH_SCALED, 0, TILE_WIDTH_SCALED, length_of_row * TILE_HEIGHT_SCALED});
			else
				painter.drawRect(QRect{0, y * TILE_HEIGHT_SCALED, length_of_row * TILE_WIDTH_SCALED, TILE_HEIGHT_SCALED});
		}
	);

	brush.setColor(Qt::white);
	painter.setBrush(brush);

	// Set all bits of the pixels in the selection to 1.
	// This alternates, creating a flashing effect.
	do_visible_tiles(
		[this, &painter](const int tile_index, const QRect &rect)
		{
			if (selected[tile_index] && selection_flip_flop)
				painter.drawRect(rect.marginsAdded(QMargins(4, 4, 4, 4)));
		}
	);

	painter.setCompositionMode(QPainter::RasterOp_SourceXorDestination);

	do_visible_tiles(
		[this, &painter](const int tile_index, const QRect &rect)
		{
			// XOR the selection.
			if (selected[tile_index])
				painter.drawRect(rect);

			// XOR the selection with the tile's pixmap.
			painter.drawPixmap(rect, tile_manager.pixmaps(tile_index, palette_line, TileManager::PixmapType::WITH_BACKGROUND));
		}
	);
}

void TileViewer::mousePressEvent(QMouseEvent* const event)
{
	QWidget::mousePressEvent(event);

	int tiles_per_row, total_rows;
	getGridDimensions(tiles_per_row, total_rows);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	const int tile_x = event->x() / TILE_WIDTH_SCALED;
	const int tile_y = event->y() / TILE_HEIGHT_SCALED;
#else
	const int tile_x = event->position().x() / TILE_WIDTH_SCALED;
	const int tile_y = event->position().y() / TILE_HEIGHT_SCALED;
#endif

	const int tile_index = m_scroll + tile_y * tiles_per_row + tile_x;

	if (tile_index >= selected.size())
		return;

	clearSelection();
	setSelection(false,
		[tile_index](QVector<bool> &selection)
		{
			selection[tile_index] = true;
		}
	);

	emit tileSelected(tile_index);
}
