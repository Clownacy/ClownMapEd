#include "colour-button.h"

#include <QBrush>
#include <QtGlobal>
#include <QPainter>

ColourButton::ColourButton()
	: m_colour(Qt::green)
{

}

void ColourButton::paintEvent(QPaintEvent* const event)
{
	Q_UNUSED(event);

	QPainter painter(this);
	QBrush brush;
	brush.setColor(this->m_colour);
	brush.setStyle(Qt::BrushStyle::SolidPattern);
	painter.setBrush(brush);
	painter.setPen(Qt::NoPen);
	painter.drawRect(QRect(QPoint(0, 0), this->size()));
}
