#ifndef SPRITE_MAPPINGS_H
#define SPRITE_MAPPINGS_H

#include <QFile>
#include <QVector>

#include "dynamic-pattern-load-cues.h"
#include "sprite-frame.h"

struct SpriteMappings
{
	static SpriteMappings fromFile(QFile &file);

	bool applyDPLCs(const DynamicPatternLoadCues &dplcs);
	DynamicPatternLoadCues removeDPLCs();

	QVector<SpriteFrame> frames;
};

#endif // SPRITE_MAPPINGS_H
