#include "sprite-frame.h"

#include <climits>

#include <QDataStream>

#include "read-stream.h"

SpriteFrame::SpriteFrame()
{

}

void SpriteFrame::fromDataStream(QDataStream &stream)
{
	const unsigned int total_pieces = Read<quint16>(stream);
	pieces.resize(total_pieces);
	pieces.squeeze();

	x1 = INT_MAX;
	x2 = INT_MIN;
	y1 = INT_MAX;
	y2 = INT_MIN;

	for (unsigned int current_piece = 0; current_piece < total_pieces; ++current_piece)
	{
		SpritePiece &piece = pieces[current_piece];

		piece.fromDataStream(stream);

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
}

void SpriteFrame::draw(QPainter &painter, const TilePixmaps &tile_pixmaps, int x_offset, int y_offset) const
{
	for (auto &piece : pieces)
		piece.draw(painter, tile_pixmaps, x_offset, y_offset);
}
