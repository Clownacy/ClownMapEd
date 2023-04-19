#include "palette-editor.h"

#include <QColorDialog>
#include <QSizePolicy>

PaletteEditor::PaletteEditor(SignalWrapper<Palette> &palette)
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

						this->palette.modify(
							[line, index, &selected_colour](Palette &palette)
							{
								palette.lines[line].colours[index] = selected_colour;
							}
						);
					}
				}
			);

			button.setColour(palette->lines[line].colours[index].toQColor224());

			button.setFixedSize(13, 13);

			grid_layout.addWidget(&button, index, line);
		}
	}

	connect(&palette, &SignalWrapper<Palette>::modified, this,
		[this]()
		{
			for (int line = 0; line < Palette::TOTAL_LINES; ++line)
				for (int index = 0; index < Palette::COLOURS_PER_LINE; ++index)
					buttons[line][index].setColour(this->palette->lines[line].colours[index].toQColor224());

			update();
		}
	);
}

void PaletteEditor::setButtonColour(const int palette_line, const int palette_index, const QColor &colour)
{
	buttons[palette_line][palette_index].setColour(colour);
}
