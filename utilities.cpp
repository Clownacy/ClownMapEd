#include "utilities.h"

namespace Utilities
{

QSizeF GetDPIScale(const QWidget* const widget)
{
	const auto dpi_to_scale = [](const int dpi)
	{
		return qRound((qreal)dpi / 96);
	};

	return QSizeF(dpi_to_scale(widget->logicalDpiX()), dpi_to_scale(widget->logicalDpiY()));
}

}
