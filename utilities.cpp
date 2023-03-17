#include "utilities.h"

namespace Utilities
{

QSizeF GetDPIScale(const QWidget* const widget)
{
	return QSizeF(qRound((qreal)widget->logicalDpiX() / 96), qRound((qreal)widget->logicalDpiY() / 96));
}

}
