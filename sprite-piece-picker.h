#ifndef SPRITE_PIECE_PICKER_H
#define SPRITE_PIECE_PICKER_H

#include <array>

#include <QGridLayout>
#include <QWidget>

#include "sprite-piece-button.h"
#include "tile-manager.h"

class SpritePiecePicker : public QWidget
{
	Q_OBJECT

public:
	SpritePiecePicker(TileManager &tile_manager);

	void setBackgroundColour(const QColor &colour);

	void setPaletteLine(const int palette_line)
	{
		this->palette_line = palette_line;
		updateSpritePieces();
	}

public slots:
	void setSelectedTile(const int tile_index)
	{
		this->tile_index = tile_index;
		updateSpritePieces();
	}

private:
	void updateSpritePieces();

	const TileManager &tile_manager;
	QGridLayout grid_layout;
	std::array<SpritePieceButton, 4 * 4> buttons;
	int tile_index = 0;
	int palette_line = 0;
};

#endif // SPRITE_PIECE_PICKER_H
