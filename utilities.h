#ifndef UTILITIES_H
#define UTILITIES_H

#include <array>

#include <QSizeF>
#include <QString>
#include <QWidget>
#include <QtMath>

namespace Utilities
{

QSizeF GetDPIScale(const QWidget *widget);

template <typename T>
inline T DivideCeiling(const T a, const T b)
{
	return (a + (b - 1)) / b;
}

template <typename T>
QString IntegerToZeroPaddedHexQString(const T integer)
{
	return QStringLiteral("%1").arg(integer, sizeof(integer) * 2, 0x10, QLatin1Char('0')).toUpper();
}

std::array<qreal, 3> QColorTosRGB(const QColor &colour);
std::array<qreal, 3> sRGBToLinearRGB(const std::array<qreal, 3> &srgb);
std::array<qreal, 3> LinearRGBToXYZ(const std::array<qreal, 3> &linear_rgb);
std::array<qreal, 3> XYZToLAB(const std::array<qreal, 3> &xyz);

inline std::array<qreal, 3> QColorToLinearRGB(const QColor &colour)
{
	return sRGBToLinearRGB(QColorTosRGB(colour));
}

inline std::array<qreal, 3> QColorToXYZ(const QColor &colour)
{
	return LinearRGBToXYZ(QColorToLinearRGB(colour));
}

inline std::array<qreal, 3> QColorToLAB(const QColor &colour)
{
	return XYZToLAB(QColorToXYZ(colour));
}

inline std::array<qreal, 3> sRGBToXYZ(const std::array<qreal, 3> &colour)
{
	return LinearRGBToXYZ(sRGBToLinearRGB(colour));
}

inline std::array<qreal, 3> sRGBToLAB(const std::array<qreal, 3> &colour)
{
	return XYZToLAB(sRGBToXYZ(colour));
}

inline std::array<qreal, 3> LinearRGBToLAB(const std::array<qreal, 3> &colour)
{
	return XYZToLAB(LinearRGBToXYZ(colour));
}

}

#endif // UTILITIES_H
