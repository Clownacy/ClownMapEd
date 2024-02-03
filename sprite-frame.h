#ifndef SPRITE_FRAME_H
#define SPRITE_FRAME_H

#include <functional>
#include <optional>

#include <QRect>
#include <QVector>

#include "libsonassmd/sprite-frame.h"

#include "sprite-piece.h"
#include "tile-manager.h"

typedef libsonassmd::SpriteFrame SpriteFrame;

void draw(const SpriteFrame &frame, QPainter &painter, bool hide_duplicate_tiles, const TileManager &tile_manager, TileManager::PixmapType effect, int starting_palette_line = 0, int x_offset = 0, int y_offset = 0, const std::optional<std::pair<int, TileManager::PixmapType>> &selected_piece = std::nullopt);
QRect calculateRect(const SpriteFrame &frame);
QVector<SpritePiece::Tile> getUniqueTiles(const SpriteFrame &frame);
void iteratePieces(const SpriteFrame &frame, const std::function<void(const SpritePiece&)> &callback);

#endif // SPRITE_FRAME_H
