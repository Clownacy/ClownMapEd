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

	const QPixmap& pixmap() const
	{
		return m_pixmap;
	}

	void changePalette(const Palette &palette);

private:
	void regeneratePixmap(const Palette &palette);

	std::array<uchar, 8 * 8 / 2> m_bytes;
	QPixmap m_pixmap;
};

#endif // TILE_H
