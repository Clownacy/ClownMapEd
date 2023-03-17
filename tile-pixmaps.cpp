#include "tile-pixmaps.h"

TilePixmaps::TilePixmaps(const QVector<unsigned char>* const tile_bytes, const Palette* const palette)
	: tiles_bytes(tile_bytes)
    , palette(palette)
{
	qint16 invalid_pixmap_raw_data[8][8];

	for (unsigned int y = 0; y < 8; ++y)
		for (unsigned int x = 0; x < 8; ++x)
			invalid_pixmap_raw_data[y][x] = 0;

	for (unsigned int i = 0; i < 8; ++i)
	{
		const unsigned int colour = 0xFF00;

		invalid_pixmap_raw_data[0][i] = colour;
		invalid_pixmap_raw_data[7][i] = colour;
		invalid_pixmap_raw_data[i][0] = colour;
		invalid_pixmap_raw_data[i][7] = colour;
		invalid_pixmap_raw_data[i][i] = colour;
		invalid_pixmap_raw_data[i][i ^ 7] = colour;
	}

	invalid_pixmap = QPixmap::fromImage(QImage(reinterpret_cast<uchar*>(invalid_pixmap_raw_data), 8, 8, QImage::Format::Format_ARGB4444_Premultiplied));

	regenerate();
}

void TilePixmaps::regenerate()
{
	if (tiles_bytes == nullptr || palette == nullptr)
		return;

	pixmaps.resize(tiles_bytes->size() / (8 * 8 / 2));

	for (std::size_t current_pixmap = 0; current_pixmap < pixmaps.size(); ++current_pixmap)
	{
		const unsigned char *tile_bytes = &(*tiles_bytes)[current_pixmap * (8 * 8 / 2)];
		qint16 pixmap_data[8 * 8];

		for (unsigned int i = 0; i < 8 * 8; ++i)
		{
			const unsigned int palette_index = (tile_bytes[i >> 1] >> (~(i << 2) & 4)) & 0xF;
			if (palette_index == 0)
			{
				pixmap_data[i] = 0;
			}
			else
			{
				const Palette::Colour colour = palette->getColour(0, palette_index);

				pixmap_data[i] = (0xF << 12) | ((colour.red >> 4) << 8) | ((colour.green >> 4) << 4) | ((colour.blue >> 4) << 0);
			}
		}

		pixmaps[current_pixmap] = QPixmap::fromImage(QImage(reinterpret_cast<uchar*>(pixmap_data), 8, 8, QImage::Format::Format_ARGB4444_Premultiplied));
	}

	emit regenerated();
}
