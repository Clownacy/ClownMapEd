#include "sprite-frame.h"

#include <climits>
#include <sstream>

#include <QSet>

void toQTextStream(const SpriteFrame &frame, QTextStream &stream, const SpritePiece::Format format)
{
	// TODO: Don't do this.
	std::stringstream string_stream;
	frame.toStream(string_stream, format);
	stream << string_stream.str().c_str();
}

void draw(const SpriteFrame &frame, QPainter &painter, bool hide_duplicate_tiles, const TileManager &tile_manager, const TileManager::PixmapType effect, const int starting_palette_line, const int x_offset, const int y_offset, const std::optional<std::pair<int, TileManager::PixmapType>> &selected_piece)
{
	const auto draw_tile = [&painter, &tile_manager, starting_palette_line, x_offset, y_offset](const SpritePiece::Tile tile, const TileManager::PixmapType effect)
	{
		SpritePiece::Tile adjusted_tile = tile;

		adjusted_tile.x += x_offset;
		adjusted_tile.y += y_offset;
		adjusted_tile.palette_line += starting_palette_line;
		adjusted_tile.palette_line %= Palette::TOTAL_LINES;

		draw(adjusted_tile, painter, tile_manager, effect);
	};

	QSet<int> recorded_tiles;

	iteratePieces(frame,
		[&frame, &draw_tile, &recorded_tiles, effect, hide_duplicate_tiles, &selected_piece](const SpritePiece &piece)
		{
			const auto this_piece_index = &piece - frame.pieces.data();
			const bool is_selected_piece = selected_piece && this_piece_index == selected_piece->first;

			iterateTiles(piece,
				[&draw_tile, &recorded_tiles, effect, hide_duplicate_tiles, &selected_piece, is_selected_piece](const SpritePiece::Tile &tile)
				{
					if (hide_duplicate_tiles)
					{
						if (recorded_tiles.contains(tile.index))
							return;

						recorded_tiles.insert(tile.index);
					}

					draw_tile(tile, is_selected_piece ? selected_piece->second : effect);
				}
			);
		}
	);
}

QRect calculateRect(const SpriteFrame &frame)
{
	int x1 = INT_MAX;
	int x2 = INT_MIN;
	int y1 = INT_MAX;
	int y2 = INT_MIN;

	for (auto &piece : frame.pieces)
	{
		const QRect rect = calculateRect(piece);

		x1 = qMin(x1, rect.left());
		x2 = qMax(x2, rect.right());
		y1 = qMin(y1, rect.top());
		y2 = qMax(y2, rect.bottom());
	}

	if (x1 == INT_MAX)
		x1 = 0;
	if (x2 == INT_MIN)
		x2 = 0;
	if (y1 == INT_MAX)
		y1 = 0;
	if (y2 == INT_MIN)
		y2 = 0;

	return QRect(QPoint(x1, y1), QPoint(x2, y2));
}

void iteratePieces(const SpriteFrame &frame, const std::function<void(const SpritePiece&)> &callback)
{
	// Must draw in reverse order.
	for (uint i = 0; i < 2; ++i)
		for (auto piece = frame.pieces.crbegin(); piece != frame.pieces.crend(); ++piece)
			if (piece->priority == (i != 0))
				callback(*piece);
}
