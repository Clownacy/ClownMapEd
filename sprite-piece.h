#ifndef SPRITEPIECE_H
#define SPRITEPIECE_H

#include <QDataStream>

class SpritePiece
{
public:
	SpritePiece();

	void fromDataStream(QDataStream &stream);

	int x = 0;
	int y = 0;
	int width = 1;
	int height = 1;
	bool priority = false;
	unsigned int palette_line = 0;
	bool y_flip = false;
	bool x_flip = false;
	unsigned int tile_index = 0;
};

#endif // SPRITEPIECE_H
