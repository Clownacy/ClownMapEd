#ifndef PALETTE_H
#define PALETTE_H

#include <array>

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

	std::array<std::array<QColor, COLOURS_PER_LINE>, TOTAL_LINES> colours;
};

#endif // PALETTE_H
