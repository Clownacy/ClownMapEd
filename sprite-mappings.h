#ifndef SPRITE_MAPPINGS_H
#define SPRITE_MAPPINGS_H

#include <QPainter>
#include <QString>
#include <QVector>

#include "sprite-piece.h"
#include "tile-pixmaps.h"

class SpriteMappings
{
public:
	struct Frame
	{
		unsigned int total_pieces;
		SpritePiece *pieces;
		int x1, x2, y1, y2;
	};

	SpriteMappings();

	void loadFromFile(const QString &file_path);
	void paintPiece(QPainter &painter, const TilePixmaps &tile_pixmaps, const SpritePiece &piece, int x_offset = 0, int y_offset = 0) const;
	void paintFrame(QPainter &painter, const TilePixmaps &tile_pixmaps, const Frame &frame, int x_offset = 0, int y_offset = 0) const;

	QVector<Frame> frames;

private:

	QVector<SpritePiece> pieces;
};

#endif // SPRITE_MAPPINGS_H
