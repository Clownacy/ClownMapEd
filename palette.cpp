#include "palette.h"

#include <QFile>

#include "data-stream.h"

void Palette::reset()
{
	for (uint line = 0; line < TOTAL_LINES; ++line)
	{
		for (uint index = 0; index < COLOURS_PER_LINE; ++index)
		{
			const uint intensity_1 = index >= 8 ? ~index & 7 : index;
			// Fun fact: those three bizarre colours at the bottom of SonMapEd's default
			// palette are the result of a bug: one of the colour channels underflow.
			// This bug has been recreated here.
			const uint intensity_2 = index >= 8 ? (~index - 1) & 7 : index;

			uint colour = 0xF000;

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

			lines[line].colours[index] = colour;
		}
	}
}

void Palette::toDataStream(DataStream &stream, const int starting_palette_line, const int ending_palette_line) const
{
	for (int line = starting_palette_line; line < ending_palette_line; ++line)
		lines[line].toDataStream(stream);
}

void Palette::fromDataStream(DataStream &stream, const int starting_palette_line)
{
	for (int line = starting_palette_line; line < TOTAL_LINES; ++line)
		lines[line].fromDataStream(stream);
}

void Palette::Line::toDataStream(DataStream &stream) const
{
	for (auto &colour : colours)
		colour.toDataStream(stream);
}

void Palette::Line::fromDataStream(DataStream &stream)
{
	for (auto &colour : colours)
		colour.fromDataStream(stream);
}

Palette::Line::Colour& Palette::Line::Colour::operator=(const QColor &colour)
{
	const uint red = (static_cast<uint>(colour.red()) >> 5) & 7;
	const uint green = (static_cast<uint>(colour.green()) >> 5) & 7;
	const uint blue = (static_cast<uint>(colour.blue()) >> 5) & 7;

	this->colour = (blue << 9) | (green << 5) | (red << 1);

	return *this;
}

QColor Palette::Line::Colour::toQColor(const std::function<uint(uint)> &callback) const
{
	// This isn't exactly what SonMapEd does:
	// For some reason, SonMapEd treats the colours as 4-bit-per-channel,
	// when they're really only 3-bit-per-channel.
	const uint red = (colour >> 1) & 7;
	const uint green = (colour >> 5) & 7;
	const uint blue = (colour >> 9) & 7;

	return QColor(callback(red), callback(green), callback(blue));
}

QColor Palette::Line::Colour::toQColor224() const
{
	return toQColor([](const uint colour_channel){return colour_channel << 5;});
}

QColor Palette::Line::Colour::toQColor256() const
{
	return toQColor([](const uint colour_channel){return colour_channel << 5 | colour_channel << 2 | colour_channel >> 1;});
}
void Palette::Line::Colour::toDataStream(DataStream &stream) const
{
	const auto original_byte_order = stream.byteOrder();
	stream.setByteOrder(DataStream::BigEndian);

	stream.write<quint16>(colour);

	stream.setByteOrder(original_byte_order);
}

void Palette::Line::Colour::fromDataStream(DataStream &stream)
{
	if (stream.status() != DataStream::Status::Ok)
		return;

	const auto original_byte_order = stream.byteOrder();
	stream.setByteOrder(DataStream::BigEndian);

	colour = stream.read<quint16>();

	stream.setByteOrder(original_byte_order);
}
