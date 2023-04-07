#ifndef SPRITE_MAPPINGS_H
#define SPRITE_MAPPINGS_H

#include <QFile>
#include <QVector>

#include "sprite-frame.h"

struct SpriteMappings
{
	static SpriteMappings fromFile(QFile &file);

	QVector<SpriteFrame> frames;
};

#endif // SPRITE_MAPPINGS_H
