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

	static constexpr int TOTAL_LINES = 4;
	static constexpr int COLOURS_PER_LINE = 16;

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

	QColor colours[TOTAL_LINES][COLOURS_PER_LINE];
};

#endif // PALETTE_H
