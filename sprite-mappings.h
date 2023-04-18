#ifndef SPRITE_MAPPINGS_H
#define SPRITE_MAPPINGS_H

#include <QFile>
#include <QVector>

#include "data-stream.h"
#include "dynamic-pattern-load-cues.h"
#include "sprite-frame.h"

struct SpriteMappings
{
	void fromFile(QFile &file, SpritePiece::Format format);
	void toDataStream(DataStream &stream, SpritePiece::Format format) const;

	bool applyDPLCs(const DynamicPatternLoadCues &dplcs);
	DynamicPatternLoadCues removeDPLCs();

	QVector<SpriteFrame> frames;
};

#endif // SPRITE_MAPPINGS_H
