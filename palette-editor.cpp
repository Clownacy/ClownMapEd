#include "palette-editor.h"

#include <QColorDialog>
#include <QSizePolicy>

PaletteEditor::PaletteEditor(Palette &palette)
	: palette(palette)
{
	setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum));
	setLayout(&grid_layout);
	grid_layout.setSpacing(0);
	grid_layout.setContentsMargins(0, 0, 0, 0);

	for (int line = 0; line < Palette::TOTAL_LINES; ++line)
	{
		for (int index = 0; index < Palette::COLOURS_PER_LINE; ++index)
		{
			ColourButton &button = buttons[line][index];

			connect(&button, &ColourButton::clicked, this,
				[this, line, index]()
				{
					ColourButton &button = *static_cast<ColourButton*>(sender());

					const QColor selected_colour = QColorDialog::getColor(button.colour(), this, "Select Colour");

					if (selected_colour.isValid())
					{
						button.setColour(selected_colour);

						this->palette.setColour(line, index, selected_colour);
					}
				}
			);

			button.setColour(palette.colour(line, index));

			button.setFixedSize(20, 20);

			grid_layout.addWidget(&button, index, line);
		}
	}

	connect(&palette, &Palette::changed, this,
		[this]()
		{
			for (int line = 0; line < Palette::TOTAL_LINES; ++line)
				for (int index = 0; index < Palette::COLOURS_PER_LINE; ++index)
					buttons[line][index].setColour(this->palette.colour(line, index));
		}
	);
}

void PaletteEditor::setButtonColour(const int palette_line, const int palette_index, const QColor &colour)
{
	buttons[palette_line][palette_index].setColour(colour);
}
