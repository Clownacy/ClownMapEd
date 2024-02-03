#include "sprite-piece.h"

#include <sstream>

void toQTextStream(const SpritePiece &piece, QTextStream &stream, const SpritePiece::Format format)
{
	// TODO: Don't do this.
	std::stringstream string_stream;
	piece.toStream(string_stream, format);
	stream << string_stream.str().c_str();
}

void iterateTiles(const SpritePiece &piece, const std::function<void(const SpritePiece::Tile&)> &callback)
{
	int current_tile_index = piece.tile_index;

	for (int tile_x = 0; tile_x < piece.width; ++tile_x)
	{
		const int tile_x_corrected = (piece.x_flip ? piece.width - tile_x - 1 : tile_x) * TileManager::TILE_WIDTH;

		for (int tile_y = 0; tile_y < piece.height; ++tile_y)
		{
			const int tile_y_corrected = (piece.y_flip ? piece.height - tile_y - 1 : tile_y) * TileManager::TILE_HEIGHT;

			callback({current_tile_index, piece.x + tile_x_corrected, piece.y + tile_y_corrected, piece.palette_line, piece.x_flip, piece.y_flip});

			++current_tile_index;
		}
	}
}

void draw(const SpritePiece::Tile &tile, QPainter &painter, const TileManager &tile_manager, const TileManager::PixmapType effect)
{
	const QRect rect(
		tile.x,
		tile.y,
		TileManager::TILE_WIDTH,
		TileManager::TILE_HEIGHT
	);

	painter.drawPixmap(rect, tile_manager.pixmaps(tile.index, tile.palette_line, effect).transformed(QTransform::fromScale(tile.x_flip ? -1 : 1, tile.y_flip ? -1 : 1)), QRectF(0, 0, TileManager::TILE_WIDTH, TileManager::TILE_HEIGHT));
}
