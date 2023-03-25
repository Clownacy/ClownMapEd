#ifndef DYNAMIC_PATTERN_LOAD_CUES_H
#define DYNAMIC_PATTERN_LOAD_CUES_H

#include <cstddef>

#include <QFile>
#include <QVector>

struct DynamicPatternLoadCues
{
	struct TileCopy
	{
		int start;
		int length;
	};

	struct Frame
	{
		QVector<TileCopy> copies;

		int getMappedTile(int tile_index) const;
	};

	QVector<Frame> frames;

	static DynamicPatternLoadCues fromFile(QFile &file);
};

#endif // DYNAMIC_PATTERN_LOAD_CUES_H
