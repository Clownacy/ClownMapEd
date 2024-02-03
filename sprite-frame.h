#ifndef SPRITE_FRAME_H
#define SPRITE_FRAME_H

#include <functional>
#include <istream>
#include <optional>
#include <unordered_map>
#include <utility>

#include <QImage>
#include <QRect>
#include <QSet>
#include <QTextStream>
#include <QVector>

#include "libsonassmd/sprite-frame.h"

#include "sprite-piece.h"
#include "tile-manager.h"

typedef libsonassmd::SpriteFrame SpriteFrame;

void toQTextStream(const SpriteFrame &frame, QTextStream &stream, SpritePiece::Format format);

void draw(const SpriteFrame &frame, QPainter &painter, bool hide_duplicate_tiles, const TileManager &tile_manager, TileManager::PixmapType effect, int starting_palette_line = 0, int x_offset = 0, int y_offset = 0, const std::optional<std::pair<int, TileManager::PixmapType>> &selected_piece = std::nullopt);
QRect calculateRect(const SpriteFrame &frame);

void iteratePieces(const SpriteFrame &frame, const std::function<void(const SpritePiece&)> &callback);

inline QVector<SpritePiece::Tile> getUniqueTiles(const SpriteFrame &frame)
{
	QVector<SpritePiece::Tile> tiles;
	QSet<int> recorded_tiles;

	iteratePieces(frame,
		[&recorded_tiles, &tiles](const SpritePiece &piece)
		{
			iterateTiles(piece,
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

#endif // SPRITE_FRAME_H
