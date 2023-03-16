#ifndef TILE_PIXMAPS_H
#define TILE_PIXMAPS_H

#include <cstddef>
#include <vector>

#include <QPixmap>

#include "palette.h"

class TilePixmaps
{
public:
	TilePixmaps(const std::vector<unsigned char> &tile_bytes, const Palette &palette);

	const QPixmap& operator[](const std::size_t tile_index) const
	{
		if (tile_index < this->total_pixmaps())
			return this->pixmaps[tile_index];
		else
			return this->invalid_pixmap;
	}

	std::size_t total_pixmaps() const {return this->m_total_pixmaps;}

private:
	std::size_t m_total_pixmaps;
	std::vector<QPixmap> pixmaps;
	QPixmap invalid_pixmap;
};

#endif // TILE_PIXMAPS_H
