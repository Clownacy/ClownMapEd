#include "sprite-piece.h"

SpritePiece SpritePiece::fromDataStream(DataStream &stream)
{
	SpritePiece piece;

	// This is specifically Sonic 2's mappings format.
	piece.y = stream.read<qint8>();
	const unsigned int size = stream.read<quint8>();
	piece.width = ((size >> 2) & 3) + 1;
	piece.height = ((size >> 0) & 3) + 1;
	const unsigned int art_tile = stream.read<quint16>();
	piece.priority = (art_tile & (1 << 15)) != 0;
	piece.palette_line = (art_tile >> 13) & 3;
	piece.y_flip = (art_tile & (1 << 12)) != 0;
	piece.x_flip = (art_tile & (1 << 11)) != 0;
	piece.tile_index = art_tile & 0x7FF;
	stream.read<quint16>(); // TODO - 2-player data?
	piece.x = stream.read<qint16>();

	return piece;
}

void SpritePiece::draw(QPainter &painter, const TileManager &tile_manager, const int x_offset, const int y_offset) const
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

			painter.drawPixmap(rect, tile_manager.pixmaps(current_tile_index++).transformed(flip_transform), QRectF(0, 0, 8, 8));
		}
	}
}
