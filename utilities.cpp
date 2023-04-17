#include "utilities.h"

namespace Utilities
{

QSizeF GetDPIScale(const QWidget* const widget)
{
	const auto dpi_to_scale = [](const int dpi)
	{
		return qRound(qreal(dpi) / 96);
	};

	return QSizeF(dpi_to_scale(widget->logicalDpiX()), dpi_to_scale(widget->logicalDpiY()));
}

std::array<qreal, 3> QColorTosRGB(const QColor &colour)
{
	std::array<qreal, 3> srgb;
	colour.getRgbF(&srgb[0], &srgb[1], &srgb[2]);
	return srgb;
}

// https://en.wikipedia.org/wiki/SRGB#From_sRGB_to_CIE_XYZ
std::array<qreal, 3> sRGBToLinearRGB(const std::array<qreal, 3> &srgb)
{
	std::array<qreal, 3> linear_rgb;

	for (std::size_t i = 0; i < srgb.size(); ++i)
	{
		if (srgb[i] <= qreal(0.04045))
			linear_rgb[i] = srgb[i] / qreal(12.92);
		else
			linear_rgb[i] = qPow((srgb[i] + qreal(0.055)) / qreal(1.055), qreal(2.4));
	}

	return linear_rgb;
}

// https://en.wikipedia.org/wiki/SRGB#From_sRGB_to_CIE_XYZ
std::array<qreal, 3> LinearRGBToXYZ(const std::array<qreal, 3> &linear_rgb)
{
	std::array<qreal, 3> xyz;

	xyz[0] = linear_rgb[0] * qreal(0.4124) + linear_rgb[1] * qreal(0.3576) + linear_rgb[2] * qreal(0.1805);
	xyz[1] = linear_rgb[0] * qreal(0.2126) + linear_rgb[1] * qreal(0.7152) + linear_rgb[2] * qreal(0.0722);
	xyz[2] = linear_rgb[0] * qreal(0.0193) + linear_rgb[1] * qreal(0.1192) + linear_rgb[2] * qreal(0.9505);

	return xyz;
}

// https://en.wikipedia.org/wiki/CIELAB_color_space#From_CIEXYZ_to_CIELAB
std::array<qreal, 3> XYZToLAB(const std::array<qreal, 3> &xyz)
{
	const auto f = [](const qreal t)
	{
		constexpr qreal magic = qreal(6) / qreal(29);

		if (t > magic * magic * magic)
			return qPow(t, qreal(1) / qreal(3));
		else
			return t / (3 * magic * magic) + qreal(4) / qreal(29);
	};

	const qreal x_adjusted = xyz[0] / qreal(0.950489);
	const qreal y_adjusted = xyz[1] / qreal(1);
	const qreal z_adjusted = xyz[2] / qreal(1.088840);

	std::array<qreal, 3> lab;

	lab[0] = qreal(116) * f(y_adjusted) - qreal(16);
	lab[1] = qreal(500) * (f(x_adjusted) - f(y_adjusted));
	lab[2] = qreal(200) * (f(y_adjusted) - f(z_adjusted));

	return lab;
}

}
