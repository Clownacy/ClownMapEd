#include "palette.h"

#include <QFile>

#include "data-stream.h"

Palette::Palette()
{
	reset();
}

void Palette::reset()
{
	for (unsigned int line = 0; line < TOTAL_LINES; ++line)
	{
		for (unsigned int index = 0; index < COLOURS_PER_LINE; ++index)
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

	emit changed();
}

void Palette::loadFromFile(const QString &file_path)
{
	QFile file(file_path);
	if (!file.open(QFile::ReadOnly))
		return;

	DataStream stream(&file);
	stream.setByteOrder(DataStream::BigEndian);

	const qint64 total_colours = file.size() / 2;

	for (qint64 line = 0; line < qMin(static_cast<qint64>(TOTAL_LINES), total_colours / COLOURS_PER_LINE); ++line)
		for (qint64 colour = 0; colour < qMin(static_cast<qint64>(COLOURS_PER_LINE), total_colours - line * COLOURS_PER_LINE); ++colour)
			colours[line][colour] = stream.read<quint16>();

	emit changed();
}

void Palette::setColour(const int palette_line, const int palette_index, const QColor &colour)
{
	if (palette_line >= TOTAL_LINES || palette_index >= COLOURS_PER_LINE)
		return;

	colours[palette_line][palette_index] = QColorToMD(colour);

	emit changed();
}

QColor Palette::MDToQColour(const unsigned int md_colour, const std::function<unsigned int(unsigned int)> callback)
{
	// This isn't exactly what SonMapEd does:
	// For some reason, SonMapEd treats the colours as 4-bit-per-channel,
	// when they're really only 3-bit-per-channel.
	const unsigned int red = (md_colour >> 1) & 7;
	const unsigned int green = (md_colour >> 5) & 7;
	const unsigned int blue = (md_colour >> 9) & 7;

	return QColor(callback(red), callback(green), callback(blue));
}

QColor Palette::MDToQColor224(const unsigned int md_colour)
{
	return MDToQColour(md_colour, [](const unsigned int colour_channel){return colour_channel << 5;});
}

QColor Palette::MDToQColor256(const unsigned int md_colour)
{
	return MDToQColour(md_colour, [](const unsigned int colour_channel){return colour_channel << 5 | colour_channel << 2 | colour_channel >> 1;});
}

unsigned int Palette::QColorToMD(const QColor &colour)
{
	unsigned int red = (static_cast<unsigned int>(colour.red()) >> 5) & 7;
	unsigned int green = (static_cast<unsigned int>(colour.green()) >> 5) & 7;
	unsigned int blue = (static_cast<unsigned int>(colour.blue()) >> 5) & 7;

	return (blue << 9) | (green << 5) | (red << 1);
}
