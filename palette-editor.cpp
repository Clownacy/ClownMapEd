#include "palette-editor.h"

#include <QColorDialog>
#include <QSizePolicy>

PaletteEditor::PaletteEditor(Palette &palette)
	: palette(palette)
{
	setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum));
	setLayout(&grid_layout);
	grid_layout.setSpacing(0);

	for (unsigned int line = 0; line < 4; ++line)
	{
		for (unsigned int index = 0; index < 16; ++index)
		{
			ColourButton &button = buttons[line][index];

			connect(&button, &ColourButton::clicked, this,
				[this]()
				{
					ColourButton &button = *static_cast<ColourButton*>(sender());

					const QColor selected_colour = QColorDialog::getColor(button.colour(), this, "Select Colour");

					if (selected_colour.isValid())
					{
						button.setColour(selected_colour);

						const unsigned int distance = &button - &buttons[0][0];
						this->palette.setColour(distance / 16, distance % 16, selected_colour);
					}
				}
			);

			button.setColour(palette.getColourQColor(line, index));

			button.setFixedSize(20, 20);

			grid_layout.addWidget(&button, index, line);
		}
	}

	connect(&palette, &Palette::changed, this,
		[this]()
		{
			for (unsigned int line = 0; line < 4; ++line)
				for (unsigned int index = 0; index < 16; ++index)
					buttons[line][index].setColour(this->palette.getColourQColor(line, index));
		}
	);
}

void PaletteEditor::setButtonColour(unsigned int palette_line, unsigned int palette_index, const QColor &colour)
{
	buttons[palette_line][palette_index].setColour(colour);
}
