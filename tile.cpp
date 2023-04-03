#include "tile.h"

Tile::Tile(std::array<uchar, TOTAL_BYTES> &bytes, const Palette &palette)
{
	for (unsigned int line = 0; line < Palette::TOTAL_LINES; ++line)
	{
		for (unsigned int i = 0; i < WIDTH * HEIGHT; ++i)
		{
			const unsigned int palette_index = (bytes[i >> 1] >> (~(i << 2) & 4)) & 0xF;

			rgb_pixels[line][i] = palette_index == 0 ? QColor(0, 0, 0, 0) : palette.colour(line, palette_index);
		}
	}

	regeneratePixmaps(palette);
}

Tile::Tile(const std::array<std::array<QColor, Tile::WIDTH>, Tile::HEIGHT> &pixels, const Palette &palette)
{
	for (int line = 0; line < Palette::TOTAL_LINES; ++line)
		for (std::size_t i = 0; i < rgb_pixels[line].size(); ++i)
			rgb_pixels[line][i] = pixels[i / Tile::WIDTH][i % Tile::WIDTH];

	regeneratePixmaps(palette);
}

void Tile::changePalette(const Palette &palette)
{
	regeneratePixmaps(palette);
}

void Tile::regeneratePixmaps(const Palette &palette)
{
	const QColor background_colour = palette.colour(0, 0);

	const auto colour_tile = [](const std::array<QColor, Tile::WIDTH * Tile::HEIGHT> &bitmap, const std::function<QColor(QColor)> callback)
	{
		std::array<std::array<uchar, 4>, Tile::WIDTH * Tile::HEIGHT> working_bitmap;

		for (unsigned int i = 0; i < WIDTH * HEIGHT; ++i)
		{
			working_bitmap[i][0] = callback(bitmap[i]).red();
			working_bitmap[i][1] = callback(bitmap[i]).green();
			working_bitmap[i][2] = callback(bitmap[i]).blue();
			working_bitmap[i][3] = callback(bitmap[i]).alpha();
		}

		return QPixmap::fromImage(QImage(&working_bitmap[0][0], WIDTH, HEIGHT, QImage::Format::Format_RGBA8888_Premultiplied));
	};

	for (unsigned int line = 0; line < Palette::TOTAL_LINES; ++line)
	{
		m_pixmaps[line][static_cast<int>(PixmapType::NO_BACKGROUND)] = colour_tile(rgb_pixels[line],
			[](const QColor colour)
			{
				return colour;
			}
		);

		m_pixmaps[line][static_cast<int>(PixmapType::WITH_BACKGROUND)] = colour_tile(rgb_pixels[line],
			[background_colour](const QColor colour)
			{
				return colour == QColor(0, 0, 0, 0) ? background_colour : colour;
			}
		);

		m_pixmaps[line][static_cast<int>(PixmapType::TRANSPARENT)] = colour_tile(rgb_pixels[line],
			[background_colour](const QColor colour) -> QColor
			{
				if (colour.alpha() == 0)
					return colour;

				return QColor(colour.red() / 4 + background_colour.red() * 3 / 4, colour.green() / 4 + background_colour.green() * 3 / 4, colour.blue() / 4 + background_colour.blue() * 3 / 4);
			}
		);
	}
}

Tile Tile::Invalid(const Palette &palette)
{
	std::array<std::array<QColor, Tile::WIDTH>, Tile::HEIGHT> invalid_pixmap_raw_data;

	for (int y = 0; y < Tile::HEIGHT; ++y)
		for (int x = 0; x < Tile::WIDTH; ++x)
			invalid_pixmap_raw_data[y][x] = QColor(0, 0, 0, 0);

	const QColor colour = Qt::red;

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
