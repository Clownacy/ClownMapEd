#ifndef SPRITE_PIECE_BUTTON_H
#define SPRITE_PIECE_BUTTON_H

#include <QAbstractButton>

#include "sprite-piece.h"
#include "tile-manager.h"

class SpritePieceButton : public QAbstractButton
{
	Q_OBJECT

public:
	SpritePieceButton(const TileManager &tile_manager);

	const SpritePiece& sprite_piece() const { return m_sprite_piece; }
	void setSpritePiece(const SpritePiece &sprite_piece);

protected:
	void paintEvent(QPaintEvent *event) override;

private:
	const TileManager &tile_manager;
	SpritePiece m_sprite_piece;
};

#endif // SPRITE_PIECE_BUTTON_H
