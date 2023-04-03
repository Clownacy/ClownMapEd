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

void SpriteFrame::draw(QPainter &painter, const TileManager &tile_manager, const Tile::PixmapType unselected_effect, const int selected_piece_index, const Tile::PixmapType selected_effect, const int starting_palette_line, const int x_offset, const int y_offset) const
{
	// Must draw in reverse order.
	for (unsigned int i = 0; i < 2; ++i)
		for (auto piece = pieces.crbegin(); piece != pieces.crend(); ++piece)
			if (piece->priority == (i != 0))
				piece->draw(painter, tile_manager, &*piece == &pieces[selected_piece_index] ? selected_effect : unselected_effect, starting_palette_line, x_offset, y_offset);
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
