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

	const int total_tiles = tiles_bytes->size() / (8 * 8 / 2);

	tiles.clear();
	tiles.reserve(total_tiles);

	for (int current_pixmap = 0; current_pixmap < total_tiles; ++current_pixmap)
	{
		std::array<uchar, 8 * 8 / 2> tile_bytes;
		const uchar* const first_byte = &(*tiles_bytes)[current_pixmap * (8 * 8 / 2)];
		std::copy(first_byte, first_byte + (8 * 8 / 2), tile_bytes.begin());

		tiles.push_back(Tile(tile_bytes, *palette));
	}

	emit regenerated();
}
