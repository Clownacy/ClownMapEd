#ifndef DYNAMIC_PATTERN_LOAD_CUES_H
#define DYNAMIC_PATTERN_LOAD_CUES_H

#include <cstddef>

#include <QFile>
#include <QVector>

struct DynamicPatternLoadCues
{
	struct TileCopy
	{
		unsigned int start;
		unsigned int length;
	};

	struct Frame
	{
		QVector<TileCopy> copies;

		unsigned int getMappedTile(unsigned int tile_index) const;
	};

	QVector<Frame> frames;

	static DynamicPatternLoadCues fromFile(QFile &file);
};

#endif // DYNAMIC_PATTERN_LOAD_CUES_H
