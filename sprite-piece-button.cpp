#include "sprite-piece-button.h"

#include <QBrush>
#include <QtGlobal>
#include <QPainter>

#include "utilities.h"

SpritePieceButton::SpritePieceButton(const TileManager &tile_manager)
	: tile_manager(tile_manager)
{
	connect(&tile_manager, &TileManager::pixmapsChanged, this, qOverload<>(&SpritePieceButton::update));
}

void SpritePieceButton::setSpritePiece(const SpritePiece &sprite_piece)
{
	m_sprite_piece = sprite_piece;
	setFixedSize(sprite_piece.width * TileManager::TILE_WIDTH * 2, sprite_piece.height * TileManager::TILE_HEIGHT * 2);
	update();
}

void SpritePieceButton::paintEvent(QPaintEvent* const event)
{
	Q_UNUSED(event);

	QPainter painter(this);

	painter.scale(width() / (m_sprite_piece.width * TileManager::TILE_WIDTH), height() / (m_sprite_piece.height * TileManager::TILE_HEIGHT));
	draw(m_sprite_piece, painter, tile_manager, TileManager::PixmapType::WITH_BACKGROUND);
}
