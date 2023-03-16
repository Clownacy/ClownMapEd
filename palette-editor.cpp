#include "palette-editor.h"

#include <QColorDialog>
#include <QSizePolicy>

PaletteEditor::PaletteEditor(Palette &palette)
	: palette(palette)
{
	this->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum));
	this->setLayout(&this->grid_layout);
	this->grid_layout.setSpacing(0);

	for (unsigned int line = 0; line < 4; ++line)
	{
		for (unsigned int index = 0; index < 16; ++index)
		{
			ColourButton &button = this->buttons[line][index];

			connect(&button, &ColourButton::clicked, this,
				[this]()
				{
					ColourButton &button = *static_cast<ColourButton*>(this->sender());

					const QColor selected_colour = QColorDialog::getColor(button.colour(), this, "Select Colour");

					if (selected_colour.isValid())
					{
						button.setColour(selected_colour);

						const unsigned int distance = &button - &this->buttons[0][0];
						this->palette.setColour(distance / 16, distance % 16, selected_colour);
					}
				}
			);

			button.setColour(this->palette.getColourQColor(line, index));

			button.setFixedSize(20, 20);

			this->grid_layout.addWidget(&button, index, line);
		}
	}
}
