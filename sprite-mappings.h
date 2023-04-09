#ifndef SPRITE_MAPPINGS_H
#define SPRITE_MAPPINGS_H

#include <QFile>
#include <QVector>

#include "data-stream.h"
#include "dynamic-pattern-load-cues.h"
#include "sprite-frame.h"

struct SpriteMappings
{
	void fromFile(QFile &file);
	void toDataStream(DataStream &stream) const;

	bool applyDPLCs(const DynamicPatternLoadCues &dplcs);
	DynamicPatternLoadCues removeDPLCs();

	QVector<SpriteFrame> frames;
};

#endif // SPRITE_MAPPINGS_H
