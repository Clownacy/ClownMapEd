#include "tile.h"

Tile::Tile(std::array<uchar, TOTAL_BYTES> &bytes, const Palette &palette)
{
	for (unsigned int line = 0; line < Palette::TOTAL_LINES; ++line)
	{
		for (unsigned int i = 0; i < WIDTH * HEIGHT; ++i)
		{
			const unsigned int palette_index = (bytes[i >> 1] >> (~(i << 2) & 4)) & 0xF;

			rgb_pixels[line][i] = palette_index == 0 ? 0 : QColorToARGB4444(palette.colour(line, palette_index));
		}
	}

	regeneratePixmaps(palette);
}

Tile::Tile(const std::array<std::array<quint16, Tile::WIDTH>, Tile::HEIGHT> &pixels, const Palette &palette)
{
	for (int i = 0; i < Palette::TOTAL_LINES; ++i)
		std::memcpy(&rgb_pixels[i], &pixels, sizeof(pixels));

	regeneratePixmaps(palette);
}

void Tile::changePalette(const Palette &palette)
{
	regeneratePixmaps(palette);
}

quint16 Tile::QColorToARGB4444(const QColor &colour)
{
	return (0xF << 12) | ((colour.red() >> 4) << 8) | ((colour.green() >> 4) << 4) | ((colour.blue() >> 4) << 0);
}

void Tile::regeneratePixmaps(const Palette &palette)
{
	const quint16 background_colour_argb4444 = QColorToARGB4444(palette.colour(0, 0));

	const auto colour_tile = [](const std::array<quint16, Tile::WIDTH * Tile::HEIGHT> &bitmap, const std::function<quint16(quint16)> callback)
	{
		std::array<quint16, Tile::WIDTH * Tile::HEIGHT> working_bitmap;

		for (unsigned int i = 0; i < WIDTH * HEIGHT; ++i)
			working_bitmap[i] = callback(bitmap[i]);

		return QPixmap::fromImage(QImage(reinterpret_cast<uchar*>(&working_bitmap), WIDTH, HEIGHT, QImage::Format::Format_ARGB4444_Premultiplied));
	};

	for (unsigned int line = 0; line < Palette::TOTAL_LINES; ++line)
	{
		m_pixmaps[line][static_cast<int>(PixmapType::NO_BACKGROUND)] = colour_tile(rgb_pixels[line],
			[](const quint16 colour)
			{
				return colour;
			}
		);

		m_pixmaps[line][static_cast<int>(PixmapType::WITH_BACKGROUND)] = colour_tile(rgb_pixels[line],
			[background_colour_argb4444](const quint16 colour)
			{
				return colour == 0 ? background_colour_argb4444 : colour;
			}
		);

		m_pixmaps[line][static_cast<int>(PixmapType::TRANSPARENT)] = colour_tile(rgb_pixels[line],
			[background_colour_argb4444](const quint16 colour) -> quint16
			{
				if (colour == 0)
					return 0;

				const unsigned int background_colour_dimmed = (background_colour_argb4444 >> 2) & 0x333;
				const unsigned int colour_dimmed = (colour >> 2) & 0x333;

				return 0xF000 | (colour_dimmed + background_colour_dimmed * 3);
			}
		);
	}
}

Tile Tile::Invalid(const Palette &palette)
{
	std::array<std::array<quint16, Tile::WIDTH>, Tile::HEIGHT> invalid_pixmap_raw_data;

	for (int y = 0; y < Tile::HEIGHT; ++y)
		for (int x = 0; x < Tile::WIDTH; ++x)
			invalid_pixmap_raw_data[y][x] = 0;

	const quint16 colour = 0xFF00;

	for (int x = 0; x < Tile::WIDTH; ++x)
	{
		invalid_pixmap_raw_data[0][x] = colour;
		invalid_pixmap_raw_data[Tile::HEIGHT - 1][x] = colour;
	}

	for (int y = 0; y < Tile::HEIGHT; ++y)
	{
		invalid_pixmap_raw_data[y][0] = colour;
		invalid_pixmap_raw_data[y][Tile::WIDTH - 1] = colour;
	}

	if (Tile::WIDTH >= Tile::HEIGHT)
	{
		for (int x = 0; x < Tile::WIDTH; ++x)
		{
			invalid_pixmap_raw_data[x * Tile::HEIGHT / Tile::WIDTH][x] = colour;
			invalid_pixmap_raw_data[(Tile::WIDTH - x - 1) * Tile::HEIGHT / Tile::WIDTH][x] = colour;
		}
	}
	else
	{
		for (int y = 0; y < Tile::HEIGHT; ++y)
		{
			invalid_pixmap_raw_data[y][y * Tile::WIDTH / Tile::HEIGHT] = colour;
			invalid_pixmap_raw_data[y][(Tile::HEIGHT - y - 1) * Tile::WIDTH / Tile::HEIGHT] = colour;
		}
	}

	return Tile(invalid_pixmap_raw_data, palette);
}
