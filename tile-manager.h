#ifndef TILE_MANAGER_H
#define TILE_MANAGER_H

#include <cstddef>

#include <QObject>
#include <QPixmap>
#include <QVector>

#include "palette.h"
#include "tile.h"

class TileManager : public QObject
{
	Q_OBJECT

public:
	TileManager(const uchar* const tile_bytes, const std::size_t total_bytes, const Palette &palette);

	void setTiles(const uchar* const tile_bytes, const std::size_t total_bytes)
	{
		setTilesInternal(tile_bytes, total_bytes);
		regenerate();
	}

	void setPalette(const Palette &palette)
	{
		this->palette = &palette;
		regenerate();
	}

	const QPixmap& pixmaps(const int tile_index) const
	{
		if (tile_index < tiles.size())
			return tiles[tile_index].pixmap();
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
	void setTilesInternal(const uchar* const tile_bytes, const std::size_t total_bytes);

	QVector<uchar> tile_bytes;
	const Palette *palette;

	QPixmap invalid_pixmap;
	QVector<Tile> tiles;
};

#endif // TILE_MANAGER_H
