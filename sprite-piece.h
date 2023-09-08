#ifndef SPRITE_PIECE_H
#define SPRITE_PIECE_H

#include <functional>

#include <QPainter>
#include <QRect>
#include <QTextStream>

#include "data-stream.h"
#include "tile-manager.h"

struct SpritePiece
{
	enum class Format
	{
		SONIC_1,
		SONIC_2,
		SONIC_3_AND_KNUCKLES,
		MAPMACROS
	};

	struct Tile
	{
		void draw(QPainter &painter, const TileManager &tile_manager, TileManager::PixmapType effect) const;

		int index;
		int x;
		int y;
		int palette_line;
		bool x_flip;
		bool y_flip;
	};

	void fromDataStream(DataStream &stream, Format format);
	void toQTextStream(QTextStream &stream, Format format) const;

	QRect rect() const
	{
		return QRect(x, y, width * TileManager::TILE_WIDTH, height * TileManager::TILE_HEIGHT);
	}

	void iterateTiles(const std::function<void(const SpritePiece::Tile&)> &callback) const;

	void draw(QPainter &painter, const TileManager &tile_manager, const TileManager::PixmapType effect) const
	{
		iterateTiles(
			[&painter, &tile_manager, effect](const Tile &tile)
			{
				tile.draw(painter, tile_manager, effect);
			}
		);
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
