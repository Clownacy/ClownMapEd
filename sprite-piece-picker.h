#ifndef SPRITE_PIECE_PICKER_H
#define SPRITE_PIECE_PICKER_H

#include <QGridLayout>
#include <QWidget>

#include "tile-manager.h"

class SpritePiecePicker : public QWidget
{
	Q_OBJECT

public:
	SpritePiecePicker(TileManager &tile_manager);

public slots:
	void setSelectedTile(int tile_index);

private:
	const TileManager &tile_manager;
	QGridLayout grid_layout;
};

#endif // SPRITE_PIECE_PICKER_H
