#include "sprite-piece-button.h"

#include <QBrush>
#include <QtGlobal>
#include <QPainter>

SpritePieceButton::SpritePieceButton(const TileManager &tile_manager)
	: tile_manager(tile_manager)
{

}

void SpritePieceButton::setSpritePiece(const SpritePiece &sprite_piece)
{
	const QSizeF dpi_scale = Utilities::GetDPIScale(this);
	m_sprite_piece = sprite_piece;
	setFixedSize(sprite_piece.width * 8 * 1.5 * dpi_scale.width(), sprite_piece.height * 8 * 1.5 * dpi_scale.height());
}

void SpritePieceButton::paintEvent(QPaintEvent* const event)
{
	Q_UNUSED(event);

	QPainter painter(this);

	QBrush brush;
	brush.setColor(Qt::black);
	brush.setStyle(Qt::BrushStyle::SolidPattern);
	painter.setBrush(brush);
	painter.setPen(Qt::NoPen);
	painter.drawRect(QRect(QPoint(0, 0), size()));

	painter.scale(width() / (m_sprite_piece.width * 8), height() / (m_sprite_piece.height * 8));
	m_sprite_piece.draw(painter, tile_manager);
}
