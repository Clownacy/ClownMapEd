#include "tile.h"

Tile::Tile(std::array<uchar, 8 * 8 / 2> &bytes, const Palette &palette)
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
	quint16 pixmap_data[8 * 8];

	for (unsigned int line = 0; line < 4; ++line)
	{
		for (unsigned int i = 0; i < 8 * 8; ++i)
		{
			const unsigned int palette_index = (m_bytes[i >> 1] >> (~(i << 2) & 4)) & 0xF;
			if (palette_index == 0)
			{
				pixmap_data[i] = 0;
			}
			else
			{
				const QColor colour = palette.colour(line, palette_index);

				pixmap_data[i] = (0xF << 12) | ((colour.red() >> 4) << 8) | ((colour.green() >> 4) << 4) | ((colour.blue() >> 4) << 0);
			}
		}

		m_pixmaps[line] = QPixmap::fromImage(QImage(reinterpret_cast<uchar*>(pixmap_data), 8, 8, QImage::Format::Format_ARGB4444_Premultiplied));
	}
}
