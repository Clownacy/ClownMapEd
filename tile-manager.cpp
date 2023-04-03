#include "tile-manager.h"

#include <cstring>

TileManager::TileManager(const uchar* const tile_bytes, const int total_bytes, const Palette &palette)
    : palette(&palette)
	, invalid_pixmap(Tile::Invalid(palette))
{
	if (tile_bytes != nullptr)
		setTilesInternal(tile_bytes, total_bytes);

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
