#include "sprite-piece.h"

#include "read-stream.h"

SpritePiece::SpritePiece()
{

}

void SpritePiece::fromDataStream(QDataStream &stream)
{
	y = Read<qint8>(stream);
	const unsigned int size = Read<quint8>(stream);
	width = ((size >> 2) & 3) + 1;
	height = ((size >> 0) & 3) + 1;
	const unsigned int art_tile = Read<quint16>(stream);
	priority = (art_tile & (1 << 15)) != 0;
	palette_line = (art_tile >> 13) & 3;
	y_flip = (art_tile & (1 << 12)) != 0;
	x_flip = (art_tile & (1 << 11)) != 0;
	tile_index = art_tile & 0x7FF;
	Read<quint16>(stream); // TODO - 2-player data?
	x = Read<qint16>(stream);
}
