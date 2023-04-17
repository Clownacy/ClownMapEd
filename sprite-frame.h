#ifndef SPRITE_FRAME_H
#define SPRITE_FRAME_H

#include <functional>
#include <optional>
#include <unordered_map>
#include <utility>

#include <QImage>
#include <QRect>
#include <QVector>

#include "data-stream.h"
#include "sprite-piece.h"
#include "tile-manager.h"

struct SpriteFrame
{
	void fromDataStream(DataStream &stream);
	void toDataStream(DataStream &stream) const;
	int size_encoded() const
	{
		return sizeof(quint16) + SpritePiece::size_encoded * pieces.size();
	}

	void draw(QPainter &painter, bool hide_duplicate_tiles, const TileManager &tile_manager, TileManager::PixmapType effect, int starting_palette_line = 0, int x_offset = 0, int y_offset = 0, const std::optional<std::pair<int, TileManager::PixmapType>> &selected_piece = std::nullopt) const;
	QRect rect() const;

	void getUniqueTiles(QVector<SpritePiece::Tile> &tiles) const
	{
		iteratePieces(
			[&tiles](const SpritePiece &piece)
			{
				piece.getUniqueTiles(tiles);
			}
		);
	}

	QVector<SpritePiece::Tile> getUniqueTiles() const
	{
		QVector<SpritePiece::Tile> tiles;
		getUniqueTiles(tiles);
		return tiles;
	}

	QVector<SpritePiece> pieces;

private:
	void iteratePieces(const std::function<void(const SpritePiece&)> &callback) const;
};

#endif // SPRITE_FRAME_H
