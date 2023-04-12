#ifndef SPRITE_PIECE_H
#define SPRITE_PIECE_H

#include <unordered_map>

#include <QPainter>
#include <QRect>

#include "data-stream.h"
#include "tile-manager.h"

struct SpritePiece
{
	void fromDataStream(DataStream &stream);
	void toDataStream(DataStream &stream) const;
	static constexpr int size_encoded = 8;

	void draw(QPainter &painter, const TileManager &tile_manager, TileManager::PixmapType effect, int starting_palette_line = 0, int x_offset = 0, int y_offset = 0, std::unordered_map<int, bool> *recorded_tiles = nullptr) const;

	QRect rect() const
	{
		return QRect(x, y, width * TileManager::TILE_WIDTH, height * TileManager::TILE_HEIGHT);
	}

	int x;
	int y;
	int width;
	int height;
	bool priority;
	int palette_line;
	bool y_flip;
	bool x_flip;
	int tile_index;
};

#endif // SPRITE_PIECE_H
