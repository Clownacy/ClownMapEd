#ifndef PALETTE_H
#define PALETTE_H

#include <array>
#include <functional>

#include <QColor>
#include <QObject>
#include <QString>

#include "data-stream.h"

struct Palette
{
	struct Line
	{
		static constexpr int TOTAL_COLOURS = 16;

		struct Colour
		{
			Colour& operator=(const uint colour)
			{
				this->colour = colour;

				return *this;
			}

			Colour& operator=(const QColor &colour);

			QColor toQColor(const std::function<uint(uint)> &callback) const;
			QColor toQColor224() const;
			QColor toQColor256() const;

			void toDataStream(DataStream &stream) const;
			void fromDataStream(DataStream &stream);

			quint16 colour;
		};

		void toDataStream(DataStream &stream) const;
		void fromDataStream(DataStream &stream);

		std::array<Colour, TOTAL_COLOURS> colours;
	};

	static constexpr int TOTAL_LINES = 4;
	static constexpr int COLOURS_PER_LINE = Line::TOTAL_COLOURS;

	void reset();
	void toDataStream(DataStream &stream, int starting_palette_line, int ending_palette_line) const;
	void fromDataStream(DataStream &stream, int starting_palette_line);

	std::array<Line, TOTAL_LINES> lines;
};

class PaletteManager : public QObject
{
	Q_OBJECT

public:
	const Palette& palette() const
	{
		return m_palette;
	}

	const void modifyPalette(const std::function<void(Palette &palette)> &callback)
	{
		callback(m_palette);

		emit changed();
	}

signals:
	void changed();

private:
	Palette m_palette;
};

#endif // PALETTE_H
