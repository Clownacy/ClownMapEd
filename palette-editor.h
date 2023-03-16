#ifndef PALETTE_EDITOR_H
#define PALETTE_EDITOR_H

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
	ColourButton buttons[4][16];
	Palette &palette;
};

#endif // PALETTE_EDITOR_H
