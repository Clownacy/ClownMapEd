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
	TileManager(const QVector<unsigned char> *tile_bytes = nullptr, const Palette *palette = nullptr);

	void setTiles(const QVector<unsigned char> &tile_bytes)
	{
		tiles_bytes = &tile_bytes;
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
	const QVector<unsigned char> *tiles_bytes = nullptr;
	const Palette *palette = nullptr;

	QPixmap invalid_pixmap;
	QVector<Tile> tiles;
};

#endif // TILE_MANAGER_H
