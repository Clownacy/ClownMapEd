#include "sprite-frame.h"

#include <climits>

SpriteFrame SpriteFrame::fromDataStream(DataStream &stream)
{
	stream.setByteOrder(DataStream::BigEndian);

	SpriteFrame frame;

	const unsigned int total_pieces = stream.read<quint16>();
	frame.pieces.resize(total_pieces);

	for (auto &piece : frame.pieces)
		piece = SpritePiece::fromDataStream(stream);

	return frame;
}

void SpriteFrame::draw(QPainter &painter, const TileManager &tile_manager, const int x_offset, const int y_offset) const
{
	// Must draw in reverse order.
	for (auto piece = pieces.crbegin(); piece != pieces.crend(); ++piece)
		piece->draw(painter, tile_manager, x_offset, y_offset);
}

QRect SpriteFrame::rect() const
{
	int x1 = INT_MAX;
	int x2 = INT_MIN;
	int y1 = INT_MAX;
	int y2 = INT_MIN;

	for (auto &piece : pieces)
	{
		if (x1 > piece.x)
			x1 = piece.x;
		if (x2 < piece.x + piece.width * 8)
			x2 = piece.x + piece.width * 8;
		if (y1 > piece.y)
			y1 = piece.y;
		if (y2 < piece.y + piece.height * 8)
			y2 = piece.y + piece.height * 8;
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
