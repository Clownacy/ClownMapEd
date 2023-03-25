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
	static constexpr int WIDTH = 8;
	static constexpr int HEIGHT = 8;
	static constexpr int TOTAL_BYTES = WIDTH * HEIGHT / 2;

	Tile(std::array<uchar, TOTAL_BYTES> &bytes, const Palette &palette);

	void changePalette(const Palette &palette);

	const std::array<QPixmap, 4>& pixmaps() const
	{
		return m_pixmaps;
	}

private:
	void regeneratePixmaps(const Palette &palette);

	std::array<uchar, TOTAL_BYTES> m_bytes;
	std::array<QPixmap, 4> m_pixmaps;
};

#endif // TILE_H
