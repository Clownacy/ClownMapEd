#ifndef SPRITE_PIECE_BUTTON_H
#define SPRITE_PIECE_BUTTON_H

#include <QAbstractButton>
#include <QColor>

#include "sprite-piece.h"
#include "tile-manager.h"

class SpritePieceButton : public QAbstractButton
{
	Q_OBJECT

public:
	SpritePieceButton(const TileManager &tile_manager);

	const SpritePiece& sprite_piece() const { return m_sprite_piece; }
	void setSpritePiece(const SpritePiece &sprite_piece);
	void setBackgroundColour(const QColor &colour)
	{
		background_colour = colour;
		update();
	}

protected:
	void paintEvent(QPaintEvent *event) override;

private:
	const TileManager &tile_manager;
	SpritePiece m_sprite_piece;
	QColor background_colour;
};

#endif // SPRITE_PIECE_BUTTON_H
