#ifndef PALETTE_EDITOR_H
#define PALETTE_EDITOR_H

#include <array>

#include <QColor>
#include <QGridLayout>
#include <QWidget>

#include "colour-button.h"
#include "palette.h"

class PaletteEditor : public QWidget
{
	Q_OBJECT

public:
	PaletteEditor(Palette &palette);

private:
	QGridLayout grid_layout;
	std::array<std::array<ColourButton, Palette::COLOURS_PER_LINE>, Palette::TOTAL_LINES> buttons;
	Palette &palette;

private slots:
	void setButtonColour(unsigned int palette_line, unsigned int palette_index, const QColor &colour);
};

#endif // PALETTE_EDITOR_H
