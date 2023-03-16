#ifndef PALETTE_H
#define PALETTE_H

#include <QColor>
#include <QObject>
#include <QString>

class Palette : public QObject
{
	Q_OBJECT;

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
		return MDToQColor(colours[palette_line][palette_index]);
	}
	void setColour(unsigned int palette_line, unsigned int palette_index, const QColor &colour);

signals:
	void colourChanged();

private:
	static QColor MDToQColor(unsigned int md_colour);
	static unsigned int QColorToMD(const QColor &colour);

	quint16 colours[4][16];
};

#endif // PALETTE_H
