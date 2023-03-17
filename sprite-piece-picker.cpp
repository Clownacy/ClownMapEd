#include "sprite-piece-picker.h"

#include "sprite-piece-button.h"

SpritePiecePicker::SpritePiecePicker(TileManager &tile_manager)
	: tile_manager(tile_manager)
{
	setLayout(&grid_layout);
	setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

	setSelectedTile(0);
}

void SpritePiecePicker::setSelectedTile(const int tile_index)
{
	for (int y = 0; y < 4; ++y)
	{
		for (int x = 0; x < 4; ++x)
		{
			const SpritePiece piece = {0, 0, x + 1, y + 1, false, 0, false, false, tile_index};

			SpritePieceButton* const button = new SpritePieceButton(tile_manager);
			button->setSpritePiece(piece);
			grid_layout.addWidget(button, y, x);
		}
	}
}
