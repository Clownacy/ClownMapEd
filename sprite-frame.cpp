#include "sprite-frame.h"

#include <climits>

void SpriteFrame::fromDataStream(DataStream &stream)
{
	const auto original_byte_order = stream.byteOrder();
	stream.setByteOrder(DataStream::BigEndian);

	const uint total_pieces = stream.read<quint16>();
	pieces.resize(total_pieces);

	for (auto &piece : pieces)
		piece.fromDataStream(stream);

	stream.setByteOrder(original_byte_order);
}

void SpriteFrame::toDataStream(DataStream &stream) const
{
	const auto original_byte_order = stream.byteOrder();
	stream.setByteOrder(DataStream::BigEndian);

	stream.write<quint16>(pieces.size());

	for (const auto &piece : pieces)
		piece.toDataStream(stream);

	stream.setByteOrder(original_byte_order);
}

void SpriteFrame::draw(QPainter &painter, const TileManager &tile_manager, const TileManager::PixmapType effect, const int starting_palette_line, const int x_offset, const int y_offset, const std::optional<std::pair<int, TileManager::PixmapType>> &selected_piece, std::unordered_map<int, bool>* recorded_tiles) const
{
	// Must draw in reverse order.
	for (uint i = 0; i < 2; ++i)
		for (auto piece = pieces.crbegin(); piece != pieces.crend(); ++piece)
			if (piece->priority == (i != 0))
				piece->draw(painter, tile_manager, selected_piece && &*piece == &pieces[selected_piece->first] ? selected_piece->second : effect, starting_palette_line, x_offset, y_offset, recorded_tiles);
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
