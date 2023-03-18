#ifndef TILE_H
#define TILE_H

#include <array>

#include <QArrayData>
#include <QPainter>
#include <QPixmap>

#include "palette.h"

class Tile
{
public:
	Tile(std::array<uchar, 8 * 8 / 2> &bytes, const Palette &palette);

	void changePalette(const Palette &palette);

	const std::array<QPixmap, 4>& pixmaps() const
	{
		return m_pixmaps;
	}

private:
	void regeneratePixmaps(const Palette &palette);

	std::array<uchar, 8 * 8 / 2> m_bytes;
	std::array<QPixmap, 4> m_pixmaps;
};

#endif // TILE_H
