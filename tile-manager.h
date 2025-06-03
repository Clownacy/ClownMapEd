#ifndef TILE_MANAGER_H
#define TILE_MANAGER_H

#include <array>
#include <cstddef>

#include <QColor>
#include <QFile>
#include <QObject>
#include <QPixmap>
#include <QVector>

#include "libsonassmd.h"

#include "palette.h"
#include "signal-wrapper.h"

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

	TileManager(const SignalWrapper<Palette> &palette_manager);

	void unloadTiles()
	{
		tiles.clear();
		tile_pixmaps.clear();

		emit pixmapsChanged();
	}

	template<typename T>
	void loadTilesFromFile(T &&file, libsonassmd::Tiles::Format format)
	{
		tiles = Tiles(std::forward<T>(file), format);

		tile_pixmaps.resize(tiles.size()); // TODO: Reserve instead?

		regeneratePixmaps();
	}

	const QPixmap& pixmaps(const int tile_index, const int palette_line, const PixmapType type) const
	{
		if (tile_index >= tile_pixmaps.size())
			return invalid_tile_pixmaps[static_cast<std::size_t>(type)];
		else
			return tile_pixmaps[tile_index][palette_line][static_cast<std::size_t>(type)];
	}

	int total_tiles() const
	{
		return tile_pixmaps.size();
	}

	void deleteTile(int tile_index);
	void duplicateTile(int tile_index, int insert_index);
	void clearTile(int tile_index);
	void modifyTiles(const std::function<void(Tiles&)> &callback);
	auto& getTiles() const
	{
		return tiles;
	}

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
	Tiles tiles;
	QVector<std::array<std::array<QPixmap, static_cast<std::size_t>(PixmapType::MAX)>, Palette::TOTAL_LINES>> tile_pixmaps;
};

#endif // TILE_MANAGER_H
