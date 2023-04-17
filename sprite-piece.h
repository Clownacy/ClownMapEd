#ifndef SPRITE_PIECE_H
#define SPRITE_PIECE_H

#include <functional>

#include <QPainter>
#include <QRect>
#include <QVector>

#include "data-stream.h"
#include "tile-manager.h"

struct SpritePiece
{
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

	void fromDataStream(DataStream &stream);
	void toDataStream(DataStream &stream) const;
	static constexpr int size_encoded = 8;

	QRect rect() const
	{
		return QRect(x, y, width * TileManager::TILE_WIDTH, height * TileManager::TILE_HEIGHT);
	}

	void getTiles(QVector<SpritePiece::Tile> &tiles) const;
	QVector<SpritePiece::Tile> getTiles() const
	{
		QVector<SpritePiece::Tile> tiles;
		getTiles(tiles);
		return tiles;
	}

	void getUniqueTiles(QVector<SpritePiece::Tile> &tiles) const;
	QVector<SpritePiece::Tile> getUniqueTiles() const
	{
		QVector<SpritePiece::Tile> tiles;
		getUniqueTiles(tiles);
		return tiles;
	}

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

private:
	void iterateTiles(const std::function<void(const SpritePiece::Tile&)> &callback) const;
};

#endif // SPRITE_PIECE_H
