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
	struct Pixmaps
	{
		QPixmap no_background;
		QPixmap with_background;
		QPixmap transparent;
	};

	static constexpr int WIDTH = 8;
	static constexpr int HEIGHT = 8;
	static constexpr int TOTAL_BYTES = WIDTH * HEIGHT / 2;

	Tile(std::array<uchar, TOTAL_BYTES> &bytes, const Palette &palette);

	void changePalette(const Palette &palette);

	const std::array<Pixmaps, Palette::TOTAL_LINES>& pixmaps() const
	{
		return m_pixmaps;
	}

	static Tile Invalid(const Palette &palette);

private:
	Tile(const std::array<std::array<quint16, Tile::WIDTH>, Tile::HEIGHT> &pixels, const Palette &palette);

	quint16 QColorToARGB4444(const QColor &colour);
	void regeneratePixmaps(const Palette &palette);

	std::array<std::array<quint16, Tile::WIDTH * Tile::HEIGHT>, Palette::TOTAL_LINES> rgb_pixels;
	std::array<Pixmaps, Palette::TOTAL_LINES> m_pixmaps;
};

#endif // TILE_H
