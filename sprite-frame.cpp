#include "sprite-frame.h"

#include <climits>

SpriteFrame SpriteFrame::fromDataStream(DataStream &stream)
{
	SpriteFrame frame;

	const unsigned int total_pieces = stream.read<quint16>();
	frame.pieces.resize(total_pieces);

	frame.x1 = INT_MAX;
	frame.x2 = INT_MIN;
	frame.y1 = INT_MAX;
	frame.y2 = INT_MIN;

	for (unsigned int current_piece = 0; current_piece < total_pieces; ++current_piece)
	{
		SpritePiece &piece = frame.pieces[current_piece];

		piece = SpritePiece::fromDataStream(stream);

		if (frame.x1 > piece.x)
			frame.x1 = piece.x;
		if (frame.x2 < piece.x + piece.width * 8)
			frame.x2 = piece.x + piece.width * 8;
		if (frame.y1 > piece.y)
			frame.y1 = piece.y;
		if (frame.y2 < piece.y + piece.height * 8)
			frame.y2 = piece.y + piece.height * 8;
	}

	if (frame.x1 == INT_MAX)
		frame.x1 = 0;
	if (frame.x2 == INT_MIN)
		frame.x2 = 0;
	if (frame.y1 == INT_MAX)
		frame.y1 = 0;
	if (frame.y2 == INT_MIN)
		frame.y2 = 0;

	return frame;
}

void SpriteFrame::draw(QPainter &painter, const TileManager &tile_manager, const int x_offset, const int y_offset) const
{
	for (auto &piece : pieces)
		piece.draw(painter, tile_manager, x_offset, y_offset);
}
