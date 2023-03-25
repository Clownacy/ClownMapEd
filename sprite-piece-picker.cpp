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

	for (int y = 0; y < MAX_PIECE_HEIGHT; ++y)
		for (int x = 0; x < MAX_PIECE_WIDTH; ++x)
			grid_layout.addWidget(&buttons[y * MAX_PIECE_HEIGHT + x], y, x);

	setSelectedTile(0);

	for (int i = 0; i < static_cast<int>(buttons.size()); ++i)
		connect(&buttons[i], &SpritePieceButton::clicked, this, [this, i](){emit pieceSelected(i % MAX_PIECE_WIDTH + 1, i / MAX_PIECE_HEIGHT + 1);});
}

void SpritePiecePicker::setBackgroundColour(const QColor &colour)
{
	for (auto &button : buttons)
		button.setBackgroundColour(colour);
}

void SpritePiecePicker::updateSpritePieces()
{
	for (int y = 0; y < MAX_PIECE_HEIGHT; ++y)
		for (int x = 0; x < MAX_PIECE_WIDTH; ++x)
			buttons[y * MAX_PIECE_HEIGHT + x].setSpritePiece({0, 0, x + 1, y + 1, false, palette_line, false, false, tile_index});
}
