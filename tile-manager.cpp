#include "tile-manager.h"

#include <cstring>

TileManager::TileManager(const uchar* const tile_bytes, const std::size_t total_bytes, const Palette &palette)
    : palette(&palette)
{
	if (tile_bytes != nullptr)
		setTilesInternal(tile_bytes, total_bytes);

	qint16 invalid_pixmap_raw_data[8][8];

	for (unsigned int y = 0; y < 8; ++y)
		for (unsigned int x = 0; x < 8; ++x)
			invalid_pixmap_raw_data[y][x] = 0;

	for (unsigned int i = 0; i < 8; ++i)
	{
		const unsigned int colour = 0xFF00;

		invalid_pixmap_raw_data[0][i] = colour;
		invalid_pixmap_raw_data[7][i] = colour;
		invalid_pixmap_raw_data[i][0] = colour;
		invalid_pixmap_raw_data[i][7] = colour;
		invalid_pixmap_raw_data[i][i] = colour;
		invalid_pixmap_raw_data[i][i ^ 7] = colour;
	}

	invalid_pixmap = QPixmap::fromImage(QImage(reinterpret_cast<uchar*>(invalid_pixmap_raw_data), 8, 8, QImage::Format::Format_ARGB4444_Premultiplied));

	regenerate();
}

void TileManager::regenerate()
{
	const int total_tiles = tile_bytes.size() / (8 * 8 / 2);

	tiles.clear();
	tiles.reserve(total_tiles);

	for (int current_pixmap = 0; current_pixmap < total_tiles; ++current_pixmap)
	{
		std::array<uchar, 8 * 8 / 2> single_tile_bytes;
		const uchar* const first_byte = &tile_bytes[current_pixmap * (8 * 8 / 2)];
		std::copy(first_byte, first_byte + (8 * 8 / 2), single_tile_bytes.begin());

		tiles.push_back(Tile(single_tile_bytes, *palette));
	}

	emit regenerated();
}

void TileManager::setTilesInternal(const uchar* const tile_bytes, const std::size_t total_bytes)
{
	this->tile_bytes.resize(total_bytes);
	this->tile_bytes.squeeze();
	std::memcpy(this->tile_bytes.data(), tile_bytes, total_bytes);
}
