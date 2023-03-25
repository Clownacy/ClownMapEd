#ifndef TILE_MANAGER_H
#define TILE_MANAGER_H

#include <array>

#include <QObject>
#include <QPixmap>
#include <QVector>

#include "palette.h"
#include "tile.h"

class TileManager : public QObject
{
	Q_OBJECT

public:
	TileManager(const uchar* const tile_bytes, const int total_bytes, const Palette &palette);

	bool setTiles(const uchar* const tile_bytes, const int total_bytes)
	{
		if (!setTilesInternal(tile_bytes, total_bytes))
			return false;

		regenerate();

		return true;
	}

	void setPalette(const Palette &palette)
	{
		this->palette = &palette;
		regenerate();
	}

	const QPixmap& pixmaps(const int tile_index, const int palette_line) const
	{
		if (tile_index < tiles.size())
			return tiles[tile_index].pixmaps()[palette_line];
		else
			return invalid_pixmap;
	}

	const int total_tiles() const
	{
		return tiles.size();
	}

public slots:
	void regenerate();

signals:
	void regenerated();

private:
	bool setTilesInternal(const uchar* const tile_bytes, const int total_bytes);

	QVector<std::array<uchar, Tile::TOTAL_BYTES>> tile_bytes;
	const Palette *palette;

	QPixmap invalid_pixmap;
	QVector<Tile> tiles;
};

#endif // TILE_MANAGER_H
