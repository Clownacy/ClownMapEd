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
		this->tiles_bytes = &tile_bytes;
		this->regenerate();
	}

	void setPalette(const Palette &palette)
	{
		this->palette = &palette;
		this->regenerate();
	}

	const QPixmap& operator[](const std::size_t tile_index) const
	{
		if (tile_index < this->total_pixmaps())
			return this->pixmaps[tile_index];
		else
			return this->invalid_pixmap;
	}

	std::size_t total_pixmaps() const // TODO: Rename to just 'total'.
	{
		return this->pixmaps.size();
	}

public slots:
	void regenerate();

private:
	const std::vector<unsigned char> *tiles_bytes = nullptr;
	const Palette *palette = nullptr;

	std::vector<QPixmap> pixmaps;
	QPixmap invalid_pixmap;
};

#endif // TILE_PIXMAPS_H
