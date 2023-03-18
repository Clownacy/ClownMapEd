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

public slots:
	void setSelectedTile(int tile_index);

private:
	const TileManager &tile_manager;
	QGridLayout grid_layout;
	std::array<SpritePieceButton, 4 * 4> buttons;
};

#endif // SPRITE_PIECE_PICKER_H
