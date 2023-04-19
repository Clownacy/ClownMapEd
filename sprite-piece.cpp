#include "sprite-piece.h"

#include "utilities.h"

void SpritePiece::fromDataStream(DataStream &stream, const Format format)
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

	switch (format)
	{
		case Format::SONIC_1:
			x = stream.read<qint8>();
			break;

		case Format::SONIC_2:
			stream.read<quint16>(); // TODO: Actually use the 2-player data?
			// Fallthrough
		case Format::SONIC_3_AND_KNUCKLES:
			x = stream.read<qint16>();
			break;
	}

	stream.setByteOrder(original_byte_order);
}

void SpritePiece::toQTextStream(QTextStream &stream, const Format format) const
{
	// TODO: Report to the user when the coordinates are truncated!
	stream << "\tdc.b\t" << y << "\n";
	stream << "\tdc.b\t$" << Utilities::IntegerToZeroPaddedHexQString((static_cast<quint8>((static_cast<uint>(width) - 1) << 2) | (static_cast<uint>(height) - 1))) << "\n";

	const uint art_tile_upper_bits = static_cast<uint>(priority) << 15
								   | static_cast<uint>(palette_line) << 13
								   | static_cast<uint>(y_flip) << 12
								   | static_cast<uint>(x_flip) << 11;

	stream << "\tdc.w\t$" << Utilities::IntegerToZeroPaddedHexQString(static_cast<quint16>(art_tile_upper_bits | static_cast<uint>(tile_index))) << "\n";

	switch (format)
	{
		case Format::SONIC_1:
			stream << "\tdc.b\t" << x << "\n";
			break;

		case Format::SONIC_2:
			stream << "\tdc.w\t$" << Utilities::IntegerToZeroPaddedHexQString(static_cast<quint16>(art_tile_upper_bits | static_cast<uint>(tile_index) / 2)) << "\n";
			// Fallthrough
		case Format::SONIC_3_AND_KNUCKLES:
			stream << "\tdc.w\t" << x << "\n";
			break;
	}
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
