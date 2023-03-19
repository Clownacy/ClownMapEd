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

	connect(&tile_manager, &TileManager::regenerated, this, qOverload<>(&SpritePiecePicker::update));
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
