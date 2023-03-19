#include "tile-manager.h"

#include <cstring>

TileManager::TileManager(const uchar* const tile_bytes, const int total_bytes, const Palette &palette)
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

	connect(&palette, &Palette::singleColourChanged, this, &TileManager::regenerate);
	connect(&palette, &Palette::allColoursChanged, this, &TileManager::regenerate);

	regenerate();
}

void TileManager::regenerate()
{
	tiles.clear();
	tiles.reserve(tile_bytes.size() / SIZE_OF_TILE);

	for (auto &single_tile_bytes : tile_bytes)
		tiles.push_back(Tile(single_tile_bytes, *palette));

	emit regenerated();
}

bool TileManager::setTilesInternal(const uchar* const tile_bytes, const int total_bytes)
{
	// Bail if the file contains a partial tile (it probably isn't tile data).
	if (total_bytes % SIZE_OF_TILE != 0)
		return false;

	const int total_tiles = total_bytes / SIZE_OF_TILE;

	this->tile_bytes.resize(total_tiles);
	this->tile_bytes.squeeze();

	for (int i = 0; i < total_tiles; ++i)
	{
		const uchar* const single_tile_bytes = tile_bytes + i * SIZE_OF_TILE;
		std::copy(single_tile_bytes, single_tile_bytes + SIZE_OF_TILE, this->tile_bytes[i].begin());
	}

	return true;
}
