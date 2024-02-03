#ifndef SPRITE_PIECE_H
#define SPRITE_PIECE_H

#include <functional>

#include <QPainter>
#include <QRect>

#include "libsonassmd/sprite-piece.h"

#include "tile-manager.h"

typedef libsonassmd::SpritePiece SpritePiece;

void draw(const SpritePiece::Tile &tile, QPainter &painter, const TileManager &tile_manager, TileManager::PixmapType effect);

inline QRect calculateRect(const SpritePiece &piece)
{
	return QRect(piece.x, piece.y, piece.width * TileManager::TILE_WIDTH, piece.height * TileManager::TILE_HEIGHT);
}

void iterateTiles(const SpritePiece &piece, const std::function<void(const SpritePiece::Tile&)> &callback);

inline void draw(const SpritePiece &piece, QPainter &painter, const TileManager &tile_manager, const TileManager::PixmapType effect)
{
	iterateTiles(piece,
		[&painter, &tile_manager, effect](const SpritePiece::Tile &tile)
		{
			draw(tile, painter, tile_manager, effect);
		}
	);
}

#endif // SPRITE_PIECE_H
