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

			this->colours[line][index] = colour;
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
			in_stream >> this->colours[i][j];
}

Palette::Colour Palette::getColour(unsigned int palette_line, unsigned int palette_index) const
{
	Palette::Colour colour;

	const unsigned int raw_colour = this->colours[palette_line][palette_index];

	colour.red = (raw_colour >> 1) & 0x7;
	colour.green = (raw_colour >> 5) & 0x7;
	colour.blue = (raw_colour >> 9) & 0x7;

	colour.red = (colour.red << 5) | (colour.red << 2) | (colour.red >> 1);
	colour.green = (colour.green << 5) | (colour.green << 2) | (colour.green >> 1);
	colour.blue = (colour.blue << 5) | (colour.blue << 2) | (colour.blue >> 1);

	return colour;
}
