#include "palette.h"

#include <QDataStream>
#include <QFile>

Palette::Palette()
{
	for (unsigned int line = 0; line < 4; ++line)
	{
		for (unsigned int index = 0; index < 16; ++index)
		{
			const unsigned int intensity_1 = index >= 8 ? ~index & 7 : index;
			// Fun fact: those three bizarre colours at the bottom of SonMapEd's default
			// palette are the result of a bug: one of the colour channels underflow.
			// This bug has been recreated here.
			const unsigned int intensity_2 = index >= 8 ? (~index - 1) & 7 : index;

			unsigned int colour = 0xF000;

			switch (line)
			{
				case 0:
					// This is not what SonMapEd does: SonMapEd just includes a copy of the
					// primary palette line from the selected Sonic game. But I worry that
					// doing that would be a copyright infringement, so instead I use a grey
					// version of the other palette lines here.
					colour |= (intensity_1 << 9) | (intensity_2 << 5) | (intensity_2 << 1);
					break;

				case 1:
					colour |= (intensity_1 << 9) | (intensity_2 << 1);
					break;

				case 2:
					colour |= (intensity_2 << 5) | (intensity_1 << 1);
					break;

				case 3:
					colour |= (intensity_2 << 9) | (intensity_1 << 5);
					break;
			}

			colours[line][index] = colour;
		}
	}
}

void Palette::loadFromFile(const QString &file_path)
{
	QFile file(file_path);
	if (!file.open(QFile::ReadOnly))
		return;

	QDataStream in_stream(&file);
	in_stream.setByteOrder(QDataStream::BigEndian);

	const std::size_t total_colours = file.size() / 2;

	for (std::size_t i = 0; i < std::min(static_cast<std::size_t>(4), total_colours / 16); ++i)
		for (std::size_t j = 0; j < std::min(static_cast<std::size_t>(16), total_colours - i * 16); ++j)
			in_stream >> colours[i][j];

	emit changed();
}

Palette::Colour Palette::getColour(const unsigned int palette_line, const unsigned int palette_index) const
{
	Palette::Colour colour;

	const QColor qt_colour = MDToQColor(colours[palette_line][palette_index]);

	colour.red = qt_colour.red();
	colour.green = qt_colour.green();
	colour.blue = qt_colour.blue();

	return colour;
}

void Palette::setColour(const unsigned int palette_line, const unsigned int palette_index, const QColor &colour)
{
	if (palette_line >= 4 || palette_index >= 16)
		return;

	colours[palette_line][palette_index] = QColorToMD(colour);

	emit changed();
}

QColor Palette::MDToQColor(unsigned int md_colour)
{
	unsigned int red = (md_colour >> 1) & 7;
	unsigned int green = (md_colour >> 5) & 7;
	unsigned int blue = (md_colour >> 9) & 7;

	red = (red << 5) | (red << 2) | (red >> 1);
	green = (green << 5) | (green << 2) | (green >> 1);
	blue = (blue << 5) | (blue << 2) | (blue >> 1);

	return QColor(red, green, blue);
}

unsigned int Palette::QColorToMD(const QColor &colour)
{
	unsigned int red = (colour.red() >> 5) & 7;
	unsigned int green = (colour.green() >> 5) & 7;
	unsigned int blue = (colour.blue() >> 5) & 7;

	return (blue << 9) | (green << 5) | (red << 1);
}
