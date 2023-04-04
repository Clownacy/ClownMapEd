#include "tile.h"

Tile::Tile(std::array<uchar, TOTAL_BYTES> &bytes, const Palette &palette)
{
	for (unsigned int line = 0; line < Palette::TOTAL_LINES; ++line)
	{
		for (unsigned int y = 0; y < HEIGHT; ++y)
		{
			for (unsigned int x = 0; x < WIDTH; ++x)
			{
				const unsigned int pixel_index = x + y * WIDTH;
				const unsigned int palette_index = (bytes[pixel_index >> 1] >> (~(pixel_index << 2) & 4)) & 0xF;

				rgb_pixels[line][y][x] = palette_index == 0 ? QColor(0, 0, 0, 0) : palette.colour256(line, palette_index);
			}
		}
	}

	regeneratePixmaps(palette);
}

Tile::Tile(const std::array<std::array<QColor, WIDTH>, HEIGHT> &pixels, const Palette &palette)
{
	for (int line = 0; line < Palette::TOTAL_LINES; ++line)
		for (int y = 0; y < HEIGHT; ++y)
			for (int x = 0; x < WIDTH; ++x)
				rgb_pixels[line][y][x] = pixels[y][x];

	regeneratePixmaps(palette);
}

void Tile::changePalette(const Palette &palette)
{
	regeneratePixmaps(palette);
}

void Tile::regeneratePixmaps(const Palette &palette)
{
	const QColor background_colour = palette.colour256(0, 0);

	const auto colour_tile = [](const std::array<std::array<QColor, WIDTH>, HEIGHT> &bitmap, const std::function<QColor(QColor)> callback)
	{
		std::array<std::array<std::array<uchar, 4>, WIDTH>, HEIGHT> working_bitmap;

		for (int y = 0; y < HEIGHT; ++y)
		{
			for (int x = 0; x < WIDTH; ++x)
			{
				working_bitmap[y][x][0] = callback(bitmap[y][x]).red();
				working_bitmap[y][x][1] = callback(bitmap[y][x]).green();
				working_bitmap[y][x][2] = callback(bitmap[y][x]).blue();
				working_bitmap[y][x][3] = callback(bitmap[y][x]).alpha();
			}
		}

		return QPixmap::fromImage(QImage(&working_bitmap[0][0][0], WIDTH, HEIGHT, QImage::Format::Format_RGBA8888_Premultiplied));
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
	std::array<std::array<QColor, WIDTH>, HEIGHT> invalid_pixmap_raw_data;

	for (int y = 0; y < HEIGHT; ++y)
		for (int x = 0; x < WIDTH; ++x)
			invalid_pixmap_raw_data[y][x] = QColor(0, 0, 0, 0);

	const QColor colour = Qt::red;

	for (int x = 0; x < WIDTH; ++x)
	{
		invalid_pixmap_raw_data[0][x] = colour;
		invalid_pixmap_raw_data[HEIGHT - 1][x] = colour;
	}

	for (int y = 0; y < HEIGHT; ++y)
	{
		invalid_pixmap_raw_data[y][0] = colour;
		invalid_pixmap_raw_data[y][WIDTH - 1] = colour;
	}

	if (WIDTH >= HEIGHT)
	{
		for (int x = 0; x < WIDTH; ++x)
		{
			invalid_pixmap_raw_data[x * HEIGHT / WIDTH][x] = colour;
			invalid_pixmap_raw_data[(WIDTH - x - 1) * HEIGHT / WIDTH][x] = colour;
		}
	}
	else
	{
		for (int y = 0; y < HEIGHT; ++y)
		{
			invalid_pixmap_raw_data[y][y * WIDTH / HEIGHT] = colour;
			invalid_pixmap_raw_data[y][(HEIGHT - y - 1) * WIDTH / HEIGHT] = colour;
		}
	}

	return Tile(invalid_pixmap_raw_data, palette);
}
