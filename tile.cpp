#include "tile.h"

Tile::Tile(std::array<uchar, TOTAL_BYTES> &bytes, const Palette &palette)
	: m_bytes(bytes)
{
	regeneratePixmaps(palette);
}

void Tile::changePalette(const Palette &palette)
{
	regeneratePixmaps(palette);
}

void Tile::regeneratePixmaps(const Palette &palette)
{
	const auto qcolor_to_argb4444 = [](const QColor &colour)
	{
		return (0xF << 12) | ((colour.red() >> 4) << 8) | ((colour.green() >> 4) << 4) | ((colour.blue() >> 4) << 0);
	};

	quint16 pixmap_data[WIDTH * HEIGHT];

	const quint16 background_colour = qcolor_to_argb4444(palette.colour(0, 0));

	for (unsigned int alpha = 0; alpha < 2; ++alpha)
	{
		for (unsigned int line = 0; line < Palette::TOTAL_LINES; ++line)
		{
			for (unsigned int i = 0; i < WIDTH * HEIGHT; ++i)
			{
				const unsigned int palette_index = (m_bytes[i >> 1] >> (~(i << 2) & 4)) & 0xF;

				if (palette_index == 0)
					pixmap_data[i] = alpha ? 0 : background_colour;
				else
					pixmap_data[i] = qcolor_to_argb4444(palette.colour(line, palette_index));
			}

			m_pixmaps[alpha][line] = QPixmap::fromImage(QImage(reinterpret_cast<uchar*>(pixmap_data), WIDTH, HEIGHT, QImage::Format::Format_ARGB4444_Premultiplied));
		}
	}
}
