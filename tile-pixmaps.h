#ifndef TILE_PIXMAPS_H
#define TILE_PIXMAPS_H

#include <cstddef>
#include <vector>

#include <QObject>
#include <QPixmap>

#include "palette.h"

class TilePixmaps : public QObject
{
	Q_OBJECT;

public:
	TilePixmaps(const std::vector<unsigned char> *tile_bytes = nullptr, const Palette *palette = nullptr);

	void setTiles(const std::vector<unsigned char> &tile_bytes)
	{
		tiles_bytes = &tile_bytes;
		regenerate();
	}

	void setPalette(const Palette &palette)
	{
		this->palette = &palette;
		regenerate();
	}

	const QPixmap& operator[](const std::size_t tile_index) const
	{
		if (tile_index < total_pixmaps())
			return pixmaps[tile_index];
		else
			return invalid_pixmap;
	}

	std::size_t total_pixmaps() const // TODO: Rename to just 'total'.
	{
		return pixmaps.size();
	}

public slots:
	void regenerate();

signals:
	void regenerated();

private:
	const std::vector<unsigned char> *tiles_bytes = nullptr;
	const Palette *palette = nullptr;

	std::vector<QPixmap> pixmaps;
	QPixmap invalid_pixmap;
};

#endif // TILE_PIXMAPS_H
