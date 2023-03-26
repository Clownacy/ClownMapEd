#include "tile-manager.h"

#include <cstring>

TileManager::TileManager(const uchar* const tile_bytes, const int total_bytes, const Palette &palette)
    : palette(&palette)
{
	if (tile_bytes != nullptr)
		setTilesInternal(tile_bytes, total_bytes);

	std::array<std::array<quint16, Tile::WIDTH>, Tile::HEIGHT> invalid_pixmap_raw_data;

	for (int y = 0; y < Tile::HEIGHT; ++y)
		for (int x = 0; x < Tile::WIDTH; ++x)
			invalid_pixmap_raw_data[y][x] = 0;

	const quint16 colour = 0xFF00;

	for (int x = 0; x < Tile::WIDTH; ++x)
	{
		invalid_pixmap_raw_data[0][x] = colour;
		invalid_pixmap_raw_data[Tile::HEIGHT - 1][x] = colour;
	}

	for (int y = 0; y < Tile::HEIGHT; ++y)
	{
		invalid_pixmap_raw_data[y][0] = colour;
		invalid_pixmap_raw_data[y][Tile::WIDTH - 1] = colour;
	}

	if (Tile::WIDTH >= Tile::HEIGHT)
	{
		for (int x = 0; x < Tile::WIDTH; ++x)
		{
			invalid_pixmap_raw_data[x * Tile::HEIGHT / Tile::WIDTH][x] = colour;
			invalid_pixmap_raw_data[(Tile::WIDTH - x - 1) * Tile::HEIGHT / Tile::WIDTH][x] = colour;
		}
	}
	else
	{
		for (int y = 0; y < Tile::HEIGHT; ++y)
		{
			invalid_pixmap_raw_data[y][y * Tile::WIDTH / Tile::HEIGHT] = colour;
			invalid_pixmap_raw_data[y][(Tile::HEIGHT - y - 1) * Tile::WIDTH / Tile::HEIGHT] = colour;
		}
	}

	invalid_pixmap = QPixmap::fromImage(QImage(reinterpret_cast<uchar*>(&invalid_pixmap_raw_data[0][0]), Tile::WIDTH, Tile::HEIGHT, QImage::Format::Format_ARGB4444_Premultiplied));

	connect(&palette, &Palette::changed, this, &TileManager::regenerate);

	regenerate();
}

void TileManager::regenerate()
{
	tiles.clear();
	tiles.reserve(tile_bytes.size() / Tile::TOTAL_BYTES);

	for (auto &single_tile_bytes : tile_bytes)
		tiles.push_back(Tile(single_tile_bytes, *palette));

	emit regenerated();
}

bool TileManager::setTilesInternal(const uchar* const tile_bytes, const int total_bytes)
{
	// Bail if the file contains a partial tile (it probably isn't tile data).
	if (total_bytes % Tile::TOTAL_BYTES != 0)
		return false;

	const int total_tiles = total_bytes / Tile::TOTAL_BYTES;

	this->tile_bytes.resize(total_tiles);
	this->tile_bytes.squeeze();

	for (int i = 0; i < total_tiles; ++i)
	{
		const uchar* const single_tile_bytes = tile_bytes + i * Tile::TOTAL_BYTES;
		std::copy(single_tile_bytes, single_tile_bytes + Tile::TOTAL_BYTES, this->tile_bytes[i].begin());
	}

	return true;
}
