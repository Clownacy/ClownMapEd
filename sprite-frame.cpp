#include "sprite-frame.h"

#include <climits>

#include <QSet>

void SpriteFrame::fromDataStream(DataStream &stream, const SpritePiece::Format format)
{
	const auto original_byte_order = stream.byteOrder();
	stream.setByteOrder(DataStream::BigEndian);

	const int total_pieces = format == SpritePiece::Format::SONIC_1 ? stream.read<quint8>() : stream.read<quint16>();
	pieces.resize(total_pieces);

	for (auto &piece : pieces)
		piece.fromDataStream(stream, format);

	stream.setByteOrder(original_byte_order);
}

void SpriteFrame::toQTextStream(QTextStream &stream, const SpritePiece::Format format) const
{
	if (format != SpritePiece::Format::MAPMACROS)
	{
		// TODO: Report to the user when this is truncated!
		stream << "\tdc." << (format == SpritePiece::Format::SONIC_1 ? "b" : "w") << "\t" << pieces.size() << "\n\n";
	}

	for (const auto &piece : pieces)
	{
		piece.toQTextStream(stream, format);
		stream << "\n";
	}
}

void SpriteFrame::draw(QPainter &painter, bool hide_duplicate_tiles, const TileManager &tile_manager, const TileManager::PixmapType effect, const int starting_palette_line, const int x_offset, const int y_offset, const std::optional<std::pair<int, TileManager::PixmapType>> &selected_piece) const
{
	const auto draw_tile = [&painter, &tile_manager, starting_palette_line, x_offset, y_offset](const SpritePiece::Tile tile, const TileManager::PixmapType effect)
	{
		SpritePiece::Tile adjusted_tile = tile;

		adjusted_tile.x += x_offset;
		adjusted_tile.y += y_offset;
		adjusted_tile.palette_line += starting_palette_line;
		adjusted_tile.palette_line %= Palette::TOTAL_LINES;

		adjusted_tile.draw(painter, tile_manager, effect);
	};

	QSet<int> recorded_tiles;

	iteratePieces(
		[this, &draw_tile, &recorded_tiles, effect, hide_duplicate_tiles, &selected_piece](const SpritePiece &piece)
		{
			const bool is_selected_piece = selected_piece && &piece - pieces.data() == selected_piece->first;

			piece.iterateTiles(
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

QRect SpriteFrame::rect() const
{
	int x1 = INT_MAX;
	int x2 = INT_MIN;
	int y1 = INT_MAX;
	int y2 = INT_MIN;

	for (auto &piece : pieces)
	{
		const QRect rect = piece.rect();

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

void SpriteFrame::iteratePieces(const std::function<void(const SpritePiece&)> &callback) const
{
	// Must draw in reverse order.
	for (uint i = 0; i < 2; ++i)
		for (auto piece = pieces.crbegin(); piece != pieces.crend(); ++piece)
			if (piece->priority == (i != 0))
				callback(*piece);
}
