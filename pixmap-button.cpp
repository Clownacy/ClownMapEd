#include "pixmap-button.h"

#include <QtGlobal>
#include <QPainter>

void PixmapButton::paintEvent(QPaintEvent* const event)
{
	Q_UNUSED(event);

	QPainter painter(this);
	painter.drawPixmap(QRect(QPoint(0, 0), size()), m_pixmap);
}
