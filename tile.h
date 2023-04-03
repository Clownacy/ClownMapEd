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
	enum class PixmapType
	{
		NO_BACKGROUND,
		WITH_BACKGROUND,
		TRANSPARENT,

		MAX
	};

	static constexpr int WIDTH = 8;
	static constexpr int HEIGHT = 8;
	static constexpr int TOTAL_BYTES = WIDTH * HEIGHT / 2;

	Tile(std::array<uchar, TOTAL_BYTES> &bytes, const Palette &palette);

	void changePalette(const Palette &palette);

	const std::array<std::array<QPixmap, static_cast<int>(PixmapType::MAX)>, Palette::TOTAL_LINES>& pixmaps() const
	{
		return m_pixmaps;
	}

	static Tile Invalid(const Palette &palette);

private:
	Tile(const std::array<std::array<QColor, WIDTH>, HEIGHT> &pixels, const Palette &palette);

	void regeneratePixmaps(const Palette &palette);

	std::array<std::array<std::array<QColor, WIDTH>, HEIGHT>, Palette::TOTAL_LINES> rgb_pixels;
	std::array<std::array<QPixmap, static_cast<int>(PixmapType::MAX)>, Palette::TOTAL_LINES> m_pixmaps;
};

#endif // TILE_H
