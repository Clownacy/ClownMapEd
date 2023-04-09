#ifndef PALETTE_EDITOR_H
#define PALETTE_EDITOR_H

#include <array>

#include <QColor>
#include <QGridLayout>
#include <QWidget>

#include "colour-button.h"
#include "palette.h"
#include "signal-wrapper.h"

class PaletteEditor : public QWidget
{
	Q_OBJECT

public:
	PaletteEditor(SignalWrapper<Palette> &palette);

private:
	QGridLayout grid_layout;
	std::array<std::array<ColourButton, Palette::COLOURS_PER_LINE>, Palette::TOTAL_LINES> buttons;
	SignalWrapper<Palette> &palette;

private slots:
	void setButtonColour(int palette_line, int palette_index, const QColor &colour);
};

#endif // PALETTE_EDITOR_H
