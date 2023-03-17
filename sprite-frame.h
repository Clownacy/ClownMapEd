#ifndef SPRITE_FRAME_H
#define SPRITE_FRAME_H

#include <QVector>

#include "data-stream.h"
#include "sprite-piece.h"
#include "tile-manager.h"

class SpriteFrame
{
public:
	SpriteFrame();

	void fromDataStream(DataStream &stream);
	void draw(QPainter &painter, const TileManager &tile_manager, int x_offset = 0, int y_offset = 0) const;

	int x1 = 0, x2 = 0, y1 = 0, y2 = 0;

private:
	QVector<SpritePiece> pieces;
};

#endif // SPRITE_FRAME_H
