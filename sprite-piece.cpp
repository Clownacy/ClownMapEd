#include "sprite-piece.h"

#include "read-stream.h"

SpritePiece::SpritePiece()
{

}

void SpritePiece::fromDataStream(QDataStream &stream)
{
	y = Read<qint8>(stream);
	const unsigned int size = Read<quint8>(stream);
	width = ((size >> 2) & 3) + 1;
	height = ((size >> 0) & 3) + 1;
	const unsigned int art_tile = Read<quint16>(stream);
	priority = (art_tile & (1 << 15)) != 0;
	palette_line = (art_tile >> 13) & 3;
	y_flip = (art_tile & (1 << 12)) != 0;
	x_flip = (art_tile & (1 << 11)) != 0;
	tile_index = art_tile & 0x7FF;
	Read<quint16>(stream); // TODO - 2-player data?
	x = Read<qint16>(stream);
}

void SpritePiece::draw(QPainter &painter, const TilePixmaps &tile_pixmaps, int x_offset, int y_offset) const
{
	const QTransform flip_transform = QTransform::fromScale(x_flip ? -1 : 1, y_flip ? -1 : 1);

	unsigned int current_tile_index = tile_index;

	for (int tile_x = 0; tile_x < width; ++tile_x)
	{
		const int tile_x_corrected = x_flip ? width - tile_x - 1 : tile_x;

		for (int tile_y = 0; tile_y < height; ++tile_y)
		{
			const int tile_y_corrected = y_flip ? height - tile_y - 1 : tile_y;

			const QRect rect(
				x_offset + x + tile_x_corrected * 8,
				y_offset + y + tile_y_corrected * 8,
				8,
				8
			);

			painter.drawPixmap(rect, tile_pixmaps[current_tile_index++].transformed(flip_transform), QRectF(0, 0, 8, 8));
		}
	}
}
