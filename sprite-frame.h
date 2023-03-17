#ifndef SPRITE_FRAME_H
#define SPRITE_FRAME_H

#include <QVector>

#include "data-stream.h"
#include "sprite-piece.h"
#include "tile-manager.h"

struct SpriteFrame
{
	static SpriteFrame fromDataStream(DataStream &stream);

	void draw(QPainter &painter, const TileManager &tile_manager, int x_offset = 0, int y_offset = 0) const;

	int x1, x2, y1, y2;
	QVector<SpritePiece> pieces;
};

#endif // SPRITE_FRAME_H
