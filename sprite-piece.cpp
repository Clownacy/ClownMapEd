#include "sprite-piece.h"

void SpritePiece::fromDataStream(DataStream &stream)
{
	const auto original_byte_order = stream.byteOrder();
	stream.setByteOrder(DataStream::BigEndian);

	// This is specifically Sonic 2's mappings format.
	y = stream.read<qint8>();
	const uint size = stream.read<quint8>();
	width = ((size >> 2) & 3) + 1;
	height = ((size >> 0) & 3) + 1;
	const uint art_tile = stream.read<quint16>();
	priority = (art_tile & (1 << 15)) != 0;
	palette_line = (art_tile >> 13) & 3;
	y_flip = (art_tile & (1 << 12)) != 0;
	x_flip = (art_tile & (1 << 11)) != 0;
	tile_index = art_tile & 0x7FF;
	stream.read<quint16>(); // TODO - 2-player data?
	x = stream.read<qint16>();

	stream.setByteOrder(original_byte_order);
}

void SpritePiece::toDataStream(DataStream &stream) const
{
	const auto original_byte_order = stream.byteOrder();
	stream.setByteOrder(DataStream::BigEndian);

	stream.write<qint8>(y);
	stream.write<quint8>((width - 1) << 2 | height - 1);

	stream.write<quint16>(static_cast<uint>(priority) << 15
						| static_cast<uint>(palette_line) << 13
						| static_cast<uint>(y_flip) << 12
						| static_cast<uint>(x_flip) << 11
						| static_cast<uint>(tile_index));

	stream.write<quint16>(static_cast<uint>(priority) << 15
						| static_cast<uint>(palette_line) << 13
						| static_cast<uint>(y_flip) << 12
						| static_cast<uint>(x_flip) << 11
						| static_cast<uint>(tile_index) / 2);

	stream.write<qint16>(x);

	stream.setByteOrder(original_byte_order);
}

void SpritePiece::draw(QPainter &painter, const TileManager &tile_manager, const TileManager::PixmapType effect, const int starting_palette_line, const int x_offset, const int y_offset, std::unordered_map<int, bool>* const recorded_tiles) const
{
	const QTransform flip_transform = QTransform::fromScale(x_flip ? -1 : 1, y_flip ? -1 : 1);

	uint current_tile_index = tile_index;

	for (int tile_x = 0; tile_x < width; ++tile_x)
	{
		const int tile_x_corrected = x_flip ? width - tile_x - 1 : tile_x;

		for (int tile_y = 0; tile_y < height; ++tile_y)
		{
			bool skip_tile = false;

			if (recorded_tiles != nullptr)
			{
				skip_tile = (*recorded_tiles)[current_tile_index];
				(*recorded_tiles)[current_tile_index] = true;
			}

			if (!skip_tile)
			{
				const int tile_y_corrected = y_flip ? height - tile_y - 1 : tile_y;

				const QRect rect(
					x_offset + x + tile_x_corrected * TileManager::TILE_WIDTH,
					y_offset + y + tile_y_corrected * TileManager::TILE_HEIGHT,
					TileManager::TILE_WIDTH,
					TileManager::TILE_HEIGHT
				);

				painter.drawPixmap(rect, tile_manager.pixmaps(current_tile_index, (starting_palette_line + palette_line) % Palette::TOTAL_LINES, effect).transformed(flip_transform), QRectF(0, 0, TileManager::TILE_WIDTH, TileManager::TILE_HEIGHT));
			}

			++current_tile_index;
		}
	}
}
