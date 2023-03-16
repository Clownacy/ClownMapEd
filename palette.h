#ifndef PALETTE_H
#define PALETTE_H

#include <QColor>
#include <QString>

class Palette
{
public:
	struct Colour
	{
		unsigned char red;
		unsigned char green;
		unsigned char blue;
	};

	Palette();

	void loadFromFile(const QString &file_path);
	Colour getColour(unsigned int palette_line, unsigned int palette_index) const;
	QColor getColourQColor(unsigned int palette_line, unsigned int palette_index) const
	{
		const Colour colour = getColour(palette_line, palette_index);
		return QColor(colour.red, colour.green, colour.blue);
	}

private:
	quint16 colours[4][16];
};

#endif // PALETTE_H
