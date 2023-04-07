#include "colour-button.h"

#include <QBrush>
#include <QPainter>
#include <QPoint>
#include <QRect>

ColourButton::ColourButton()
	: m_colour(Qt::green)
{

}

void ColourButton::paintEvent(QPaintEvent* const event)
{
	Q_UNUSED(event);

	QPainter painter(this);
	QBrush brush;
	brush.setColor(m_colour);
	brush.setStyle(Qt::BrushStyle::SolidPattern);
	painter.setBrush(brush);
	painter.setPen(Qt::NoPen);
	painter.drawRect(QRect(QPoint(0, 0), size()));
}
