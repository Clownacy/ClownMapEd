#ifndef UTILITIES_H
#define UTILITIES_H

#include <QSizeF>
#include <QWidget>

namespace Utilities
{

QSizeF GetDPIScale(const QWidget *widget);

template <typename T>
static inline T DivideCeiling(const T a, const T b)
{
	return (a + (b - 1)) / b;
}

}

#endif // UTILITIES_H
