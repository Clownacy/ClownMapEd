#ifndef SPRITE_FRAME_H
#define SPRITE_FRAME_H

#include <optional>
#include <utility>

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

	void draw(QPainter &painter, const TileManager &tile_manager, TileManager::PixmapType effect, int starting_palette_line = 0, int x_offset = 0, int y_offset = 0, const std::optional<std::pair<int, TileManager::PixmapType>> &selected_piece = std::nullopt) const;
	QRect rect() const;

	QVector<SpritePiece> pieces;
};

#endif // SPRITE_FRAME_H
