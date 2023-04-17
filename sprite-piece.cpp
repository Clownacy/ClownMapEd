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

void SpritePiece::getTiles(QVector<SpritePiece::Tile> &tiles) const
{
	iterateTiles(
		[&tiles](const SpritePiece::Tile &tile)
		{
			tiles.push_back(tile);
		}
	);
}

void SpritePiece::getUniqueTiles(QVector<SpritePiece::Tile> &tiles) const
{
	iterateTiles(
		[&tiles](const SpritePiece::Tile &new_tile)
		{
			for (const auto &tile : qAsConst(tiles))
				if (tile.index == new_tile.index)
					return;

			tiles.push_back(new_tile);
		}
	);
}

void SpritePiece::iterateTiles(const std::function<void(const SpritePiece::Tile&)> &callback) const
{
	int current_tile_index = tile_index;

	for (int tile_x = 0; tile_x < width; ++tile_x)
	{
		const int tile_x_corrected = (x_flip ? width - tile_x - 1 : tile_x) * TileManager::TILE_WIDTH;

		for (int tile_y = 0; tile_y < height; ++tile_y)
		{
			const int tile_y_corrected = (y_flip ? height - tile_y - 1 : tile_y) * TileManager::TILE_HEIGHT;

			callback({current_tile_index, x + tile_x_corrected, y + tile_y_corrected, palette_line, x_flip, y_flip});

			++current_tile_index;
		}
	}
}

void SpritePiece::Tile::draw(QPainter &painter, const TileManager &tile_manager, const TileManager::PixmapType effect) const
{
	const QRect rect(
		x,
		y,
		TileManager::TILE_WIDTH,
		TileManager::TILE_HEIGHT
	);

	painter.drawPixmap(rect, tile_manager.pixmaps(index, palette_line, effect).transformed(QTransform::fromScale(x_flip ? -1 : 1, y_flip ? -1 : 1)), QRectF(0, 0, TileManager::TILE_WIDTH, TileManager::TILE_HEIGHT));
}
