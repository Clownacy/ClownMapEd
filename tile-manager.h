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

	const QPixmap& pixmaps(const std::size_t tile_index) const
	{
		// TODO: Stop using QVector because its developers are complete and
		// utter morons who use a plain `int` to represent the size of a vector.
		if (tile_index < static_cast<std::size_t>(tiles.size()))
			return tiles[tile_index].pixmap();
		else
			return invalid_pixmap;
	}

	const std::size_t total_tiles() const
	{
		return static_cast<std::size_t>(tiles.size());
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
