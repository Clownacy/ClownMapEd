#ifndef PALETTE_H
#define PALETTE_H

#include <QColor>
#include <QObject>
#include <QString>

class Palette : public QObject
{
	Q_OBJECT

public:
	struct Colour
	{
		unsigned char red;
		unsigned char green;
		unsigned char blue;
	};

	Palette();

	void loadFromFile(const QString &file_path);
	QColor colour(unsigned int palette_line, unsigned int palette_index) const
	{
		return colours[palette_line][palette_index];
	}
	void setColour(unsigned int palette_line, unsigned int palette_index, const QColor &colour);

signals:
	void changed();

private:
	static QColor MDToQColor(unsigned int md_colour);
	static unsigned int QColorToMD(const QColor &colour);

	QColor colours[4][16];
};

#endif // PALETTE_H
