#include "tile-manager.h"

#include <algorithm>

TileManager::TileManager(const SignalWrapper<Palette> &palette)
    : palette(*palette)
	, invalid_tile_pixmaps(createPixmaps(createInvalidTilePixmap()))
{
	connect(&palette, &SignalWrapper<Palette>::modified, this, &TileManager::regeneratePixmaps);
}

void TileManager::deleteTile(const int tile_index)
{
	tiles.erase(tiles.cbegin() + tile_index);
	tile_pixmaps.remove(tile_index);

	emit pixmapsChanged();
}

void TileManager::duplicateTile(const int tile_index, const int insert_index)
{
	tiles.insert(tiles.cbegin() + insert_index, tiles[tile_index]);
	tile_pixmaps.insert(insert_index, tile_pixmaps[tile_index]);

	emit pixmapsChanged();
}

void TileManager::clearTile(const int tile_index)
{
	std::fill(&tiles[tile_index].pixels[0][0], &tiles[tile_index].pixels[0][0] + sizeof(tiles[tile_index]), 0);
	regeneratePixmap(tile_index);

	emit pixmapsChanged();
}

void TileManager::modifyTiles(const std::function<void(Tiles&)> &callback)
{
	callback(tiles);
	regeneratePixmaps();
}

void TileManager::regeneratePixmaps()
{
	invalid_tile_pixmaps = createPixmaps(createInvalidTilePixmap());
	tile_pixmaps.resize(tiles.size());

	for (int tile_index = 0; tile_index < static_cast<int>(tiles.size()); ++tile_index)
		regeneratePixmap(tile_index);

	emit pixmapsChanged();
}

void TileManager::regeneratePixmap(const int tile_index)
{
	for (uint line = 0; line < Palette::TOTAL_LINES; ++line)
	{
		std::array<std::array<QColor, TILE_WIDTH>, TILE_HEIGHT> rgb_pixels;

		for (uint y = 0; y < TILE_HEIGHT; ++y)
		{
			for (uint x = 0; x < TILE_WIDTH; ++x)
			{
				const uint palette_index = tiles[tile_index].pixels[y][x];

				rgb_pixels[y][x] = palette_index == 0 ? QColor(0, 0, 0, 0) : palette.lines[line].colours[palette_index].toQColor224();
			}
		}

		tile_pixmaps[tile_index][line] = createPixmaps(rgb_pixels);
	}
}

std::array<QPixmap, static_cast<std::size_t>(TileManager::PixmapType::MAX)> TileManager::createPixmaps(const std::array<std::array<QColor, TILE_WIDTH>, TILE_HEIGHT> rgb_pixels)
{
	const auto createPixmap = [](const std::array<std::array<QColor, TILE_WIDTH>, TILE_HEIGHT> &bitmap, const std::function<QColor(QColor)> &callback)
	{
		std::array<std::array<std::array<uchar, 4>, TILE_WIDTH>, TILE_HEIGHT> working_bitmap;

		for (int y = 0; y < TILE_HEIGHT; ++y)
		{
			for (int x = 0; x < TILE_WIDTH; ++x)
			{
				const QColor colour = callback(bitmap[y][x]);
				working_bitmap[y][x][0] = colour.red();
				working_bitmap[y][x][1] = colour.green();
				working_bitmap[y][x][2] = colour.blue();
				working_bitmap[y][x][3] = colour.alpha();
			}
		}

		return QPixmap::fromImage(QImage(&working_bitmap[0][0][0], TILE_WIDTH, TILE_HEIGHT, QImage::Format::Format_RGBA8888_Premultiplied));
	};

	const QColor background_colour = palette.lines[0].colours[0].toQColor224();

	std::array<QPixmap, static_cast<std::size_t>(PixmapType::MAX)> pixmaps;

	pixmaps[static_cast<std::size_t>(PixmapType::NO_BACKGROUND)] = createPixmap(rgb_pixels,
		[](const QColor &colour)
		{
			return colour;
		}
	);

	pixmaps[static_cast<std::size_t>(PixmapType::WITH_BACKGROUND)] = createPixmap(rgb_pixels,
		[&background_colour](const QColor &colour)
		{
			return colour == QColor(0, 0, 0, 0) ? background_colour : colour;
		}
	);

	pixmaps[static_cast<std::size_t>(PixmapType::TRANSPARENT)] = createPixmap(rgb_pixels,
		[&background_colour](const QColor &colour)
		{
			if (colour.alpha() == 0)
				return colour;

			return QColor(colour.red() / 4 + background_colour.red() * 3 / 4, colour.green() / 4 + background_colour.green() * 3 / 4, colour.blue() / 4 + background_colour.blue() * 3 / 4);
		}
	);

	return pixmaps;
}

std::array<std::array<QColor, TileManager::TILE_WIDTH>, TileManager::TILE_HEIGHT> TileManager::createInvalidTilePixmap()
{
	std::array<std::array<QColor, TILE_WIDTH>, TILE_HEIGHT> invalid_pixmap_raw_data;

	for (int y = 0; y < TILE_HEIGHT; ++y)
		for (int x = 0; x < TILE_WIDTH; ++x)
			invalid_pixmap_raw_data[y][x] = QColor(0, 0, 0, 0);

	const QColor colour = Qt::red;

	for (int x = 0; x < TILE_WIDTH; ++x)
	{
		invalid_pixmap_raw_data[0][x] = colour;
		invalid_pixmap_raw_data[TILE_HEIGHT - 1][x] = colour;
	}

	for (int y = 0; y < TILE_HEIGHT; ++y)
	{
		invalid_pixmap_raw_data[y][0] = colour;
		invalid_pixmap_raw_data[y][TILE_WIDTH - 1] = colour;
	}

	if (TILE_WIDTH >= TILE_HEIGHT)
	{
		for (int x = 0; x < TILE_WIDTH; ++x)
		{
			invalid_pixmap_raw_data[x * TILE_HEIGHT / TILE_WIDTH][x] = colour;
			invalid_pixmap_raw_data[(TILE_WIDTH - x - 1) * TILE_HEIGHT / TILE_WIDTH][x] = colour;
		}
	}
	else
	{
		for (int y = 0; y < TILE_HEIGHT; ++y)
		{
			invalid_pixmap_raw_data[y][y * TILE_WIDTH / TILE_HEIGHT] = colour;
			invalid_pixmap_raw_data[y][(TILE_HEIGHT - y - 1) * TILE_WIDTH / TILE_HEIGHT] = colour;
		}
	}

	return invalid_pixmap_raw_data;
}
