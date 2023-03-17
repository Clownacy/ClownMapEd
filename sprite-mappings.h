#ifndef SPRITE_MAPPINGS_H
#define SPRITE_MAPPINGS_H

#include <QString>
#include <QVector>

class SpriteMappings
{
public:
	struct Piece
	{
		int x;
		int y;
		int width;
		int height;
		bool priority;
		unsigned int palette_line;
		bool y_flip;
		bool x_flip;
		unsigned int tile_index;
	};

	struct Frame
	{
		unsigned int total_pieces;
		Piece *pieces;
		int x1, x2, y1, y2;
	};

	SpriteMappings();

	void loadFromFile(const QString &file_path);

	QVector<Frame> frames;

private:
	QVector<Piece> pieces;
};

#endif // SPRITE_MAPPINGS_H
