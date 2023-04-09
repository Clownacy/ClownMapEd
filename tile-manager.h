#ifndef TILE_MANAGER_H
#define TILE_MANAGER_H

#include <algorithm>
#include <array>
#include <cstddef>

#include <QColor>
#include <QObject>
#include <QPixmap>
#include <QVector>

#include "palette.h"

class TileManager : public QObject
{
	Q_OBJECT

public:
	enum class PixmapType
	{
		NO_BACKGROUND,
		WITH_BACKGROUND,
		TRANSPARENT,

		MAX
	};

	static constexpr int TILE_WIDTH = 8;
	static constexpr int TILE_HEIGHT = 8;
	static constexpr int TILE_SIZE_IN_BYTES = TILE_WIDTH * TILE_HEIGHT / 2;

	TileManager(const PaletteManager &palette_manager);

	void unloadTiles()
	{
		tiles_bytes.clear();
		tile_pixmaps.clear();

		emit pixmapsChanged();
	}

	template<typename It>
	bool setTiles(const It &tile_bytes_first, const It &tile_bytes_last)
	{
		const auto total_bytes = tile_bytes_last - tile_bytes_first;

		// Bail if the file contains a partial tile (it probably isn't tile data).
		if (total_bytes % TILE_SIZE_IN_BYTES != 0)
			return false;

		const int total_tiles = total_bytes / TILE_SIZE_IN_BYTES;

		tiles_bytes.resize(total_tiles);

		for (int i = 0; i < total_tiles; ++i)
			std::copy(tile_bytes_first + i * TILE_SIZE_IN_BYTES, tile_bytes_first + (i + 1) * TILE_SIZE_IN_BYTES, tiles_bytes[i].data());

		tile_pixmaps.resize(tiles_bytes.size()); // TODO: Reserve instead?

		regeneratePixmaps();

		return true;
	}

	const QPixmap& pixmaps(const int tile_index, const int palette_line, const PixmapType type) const
	{
		if (tile_index >= tile_pixmaps.size())
			return invalid_tile_pixmaps[static_cast<std::size_t>(type)];
		else
			return tile_pixmaps[tile_index][palette_line][static_cast<std::size_t>(type)];
	}

	const int total_tiles() const
	{
		return tile_pixmaps.size();
	}

	void deleteTile(int tile_index);
	void duplicateTile(int tile_index, int insert_index);
	void clearTile(int tile_index);

signals:
	void pixmapsChanged();

private slots:
	void regeneratePixmaps();

private:
	void regeneratePixmap(int tile_index);

	std::array<QPixmap, static_cast<std::size_t>(PixmapType::MAX)> createPixmaps(const std::array<std::array<QColor, TILE_WIDTH>, TILE_HEIGHT> rgb_pixels);
	static std::array<std::array<QColor, TILE_WIDTH>, TILE_HEIGHT> createInvalidTilePixmap();

	const Palette &palette;

	std::array<QPixmap, static_cast<std::size_t>(PixmapType::MAX)> invalid_tile_pixmaps;
	QVector<std::array<uchar, TILE_SIZE_IN_BYTES>> tiles_bytes;
	QVector<std::array<std::array<QPixmap, static_cast<std::size_t>(PixmapType::MAX)>, Palette::TOTAL_LINES>> tile_pixmaps;
};

#endif // TILE_MANAGER_H
