#include "sprite-piece-picker.h"

SpritePiecePicker::SpritePiecePicker(TileManager &tile_manager)
	: tile_manager(tile_manager)
	, buttons{{
		{SpritePieceButton(tile_manager), SpritePieceButton(tile_manager), SpritePieceButton(tile_manager), SpritePieceButton(tile_manager)},
		{SpritePieceButton(tile_manager), SpritePieceButton(tile_manager), SpritePieceButton(tile_manager), SpritePieceButton(tile_manager)},
		{SpritePieceButton(tile_manager), SpritePieceButton(tile_manager), SpritePieceButton(tile_manager), SpritePieceButton(tile_manager)},
		{SpritePieceButton(tile_manager), SpritePieceButton(tile_manager), SpritePieceButton(tile_manager), SpritePieceButton(tile_manager)}
	}}
{
	setLayout(&grid_layout);
	setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	grid_layout.setSpacing(16);

	for (int y = 0; y < MAX_PIECE_HEIGHT; ++y)
		for (int x = 0; x < MAX_PIECE_WIDTH; ++x)
			grid_layout.addWidget(&buttons[y][x], y, x);

	setSelectedTile(0);

	for (int y = 0; y < MAX_PIECE_HEIGHT; ++y)
		for (int x = 0; x < MAX_PIECE_WIDTH; ++x)
			connect(&buttons[y][x], &SpritePieceButton::clicked, this, [this, x, y](){emit pieceSelected(x + 1, y + 1);});
}

void SpritePiecePicker::updateSpritePieces()
{
	for (int y = 0; y < MAX_PIECE_HEIGHT; ++y)
		for (int x = 0; x < MAX_PIECE_WIDTH; ++x)
			buttons[y][x].setSpritePiece(SpritePiece{0, 0, x + 1, y + 1, false, palette_line, false, false, tile_index});
}
