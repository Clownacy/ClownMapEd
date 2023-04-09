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
	PaletteEditor(PaletteManager &palette_manager);

private:
	QGridLayout grid_layout;
	std::array<std::array<ColourButton, Palette::COLOURS_PER_LINE>, Palette::TOTAL_LINES> buttons;
	PaletteManager &palette_manager;

private slots:
	void setButtonColour(int palette_line, int palette_index, const QColor &colour);
};

#endif // PALETTE_EDITOR_H
