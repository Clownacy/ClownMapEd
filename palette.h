#ifndef PALETTE_H
#define PALETTE_H

#include <array>
#include <functional>

#include <QColor>
#include <QObject>
#include <QString>

class Palette : public QObject
{
	Q_OBJECT

public:
	static constexpr int TOTAL_LINES = 4;
	static constexpr int COLOURS_PER_LINE = 16;

	Palette();

	void reset();
	void loadFromFile(const QString &file_path, const int starting_palette_line);
	QColor colour224(const int palette_line, const int palette_index) const
	{
		return MDToQColor224(colours[palette_line][palette_index]);
	}
	QColor colour256(const int palette_line, const int palette_index) const
	{
		return MDToQColor256(colours[palette_line][palette_index]);
	}
	void setColour(int palette_line, int palette_index, const QColor &colour);

signals:
	void changed();

private:
	static QColor MDToQColour(uint md_colour, const std::function<uint(uint)> &callback);
	static QColor MDToQColor224(uint md_colour);
	static QColor MDToQColor256(uint md_colour);
	static uint QColorToMD(const QColor &colour);

	std::array<std::array<quint16, COLOURS_PER_LINE>, TOTAL_LINES> colours;
};

#endif // PALETTE_H
