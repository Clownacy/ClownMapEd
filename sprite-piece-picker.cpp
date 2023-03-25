#include "sprite-piece-picker.h"

SpritePiecePicker::SpritePiecePicker(TileManager &tile_manager)
	: tile_manager(tile_manager)
	, buttons{{
		SpritePieceButton(tile_manager), SpritePieceButton(tile_manager), SpritePieceButton(tile_manager), SpritePieceButton(tile_manager),
		SpritePieceButton(tile_manager), SpritePieceButton(tile_manager), SpritePieceButton(tile_manager), SpritePieceButton(tile_manager),
		SpritePieceButton(tile_manager), SpritePieceButton(tile_manager), SpritePieceButton(tile_manager), SpritePieceButton(tile_manager),
		SpritePieceButton(tile_manager), SpritePieceButton(tile_manager), SpritePieceButton(tile_manager), SpritePieceButton(tile_manager)
	}}
{
	setLayout(&grid_layout);
	setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

	for (int y = 0; y < 4; ++y)
		for (int x = 0; x < 4; ++x)
			grid_layout.addWidget(&buttons[y * 4 + x], y, x);

	setSelectedTile(0);

	for (int i = 0; i < static_cast<int>(buttons.size()); ++i)
		connect(&buttons[i], &SpritePieceButton::clicked, this, [this, i](){emit pieceSelected(i % 4 + 1, i / 4 + 1);});
}

void SpritePiecePicker::setBackgroundColour(const QColor &colour)
{
	for (auto &button : buttons)
		button.setBackgroundColour(colour);
}

void SpritePiecePicker::updateSpritePieces()
{
	for (int y = 0; y < 4; ++y)
		for (int x = 0; x < 4; ++x)
			buttons[y * 4 + x].setSpritePiece({0, 0, x + 1, y + 1, false, palette_line, false, false, tile_index});
}
