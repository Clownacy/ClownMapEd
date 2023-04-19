#ifndef SPRITE_FRAME_H
#define SPRITE_FRAME_H

#include <functional>
#include <optional>
#include <unordered_map>
#include <utility>

#include <QImage>
#include <QRect>
#include <QSet>
#include <QTextStream>
#include <QVector>

#include "data-stream.h"
#include "sprite-piece.h"
#include "tile-manager.h"

struct SpriteFrame
{
	void fromDataStream(DataStream &stream, SpritePiece::Format format);
	void toQTextStream(QTextStream &stream, SpritePiece::Format format) const;

	void draw(QPainter &painter, bool hide_duplicate_tiles, const TileManager &tile_manager, TileManager::PixmapType effect, int starting_palette_line = 0, int x_offset = 0, int y_offset = 0, const std::optional<std::pair<int, TileManager::PixmapType>> &selected_piece = std::nullopt) const;
	QRect rect() const;

	QVector<SpritePiece::Tile> getUniqueTiles() const
	{
		QVector<SpritePiece::Tile> tiles;
		QSet<int> recorded_tiles;

		iteratePieces(
			[&recorded_tiles, &tiles](const SpritePiece &piece)
			{
				piece.iterateTiles(
					[&recorded_tiles, &tiles](const SpritePiece::Tile &tile)
					{
						if (recorded_tiles.contains(tile.index))
							return;

						recorded_tiles.insert(tile.index);

						tiles.append(tile);
					}
				);
			}
		);

		return tiles;
	}

	QVector<SpritePiece> pieces;

private:
	void iteratePieces(const std::function<void(const SpritePiece&)> &callback) const;
};

#endif // SPRITE_FRAME_H
