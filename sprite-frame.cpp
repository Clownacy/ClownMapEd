#include "sprite-frame.h"

#include <climits>

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

void SpriteFrame::toDataStream(DataStream &stream, const SpritePiece::Format format) const
{
	const auto original_byte_order = stream.byteOrder();
	stream.setByteOrder(DataStream::BigEndian);

	// TODO: Report to the user when this is truncated!
	if (format == SpritePiece::Format::SONIC_1)
		stream.write<quint8>(pieces.size());
	else
		stream.write<quint16>(pieces.size());

	for (const auto &piece : pieces)
		piece.toDataStream(stream, format);

	stream.setByteOrder(original_byte_order);
}

void SpriteFrame::draw(QPainter &painter, bool hide_duplicate_tiles, const TileManager &tile_manager, const TileManager::PixmapType effect, const int starting_palette_line, const int x_offset, const int y_offset, const std::optional<std::pair<int, TileManager::PixmapType>> &selected_piece) const
{
	QVector<SpritePiece::Tile> tiles;

	if (hide_duplicate_tiles)
	{
		iteratePieces(
			[&tiles](const SpritePiece &piece)
			{
				piece.getUniqueTiles(tiles);
			}
		);
	}
	else
	{
		iteratePieces(
			[&tiles](const SpritePiece &piece)
			{
				piece.getTiles(tiles);
			}
		);
	}

	for (const auto &tile : qAsConst(tiles))
	{
		SpritePiece::Tile adjusted_tile = tile;

		adjusted_tile.x += x_offset;
		adjusted_tile.y += y_offset;
		adjusted_tile.palette_line += starting_palette_line;
		adjusted_tile.palette_line %= Palette::TOTAL_LINES;

		adjusted_tile.draw(painter, tile_manager, effect);
	}
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
