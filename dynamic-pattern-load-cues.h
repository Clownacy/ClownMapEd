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

	QVector<QVector<TileCopy>> frames;

	static DynamicPatternLoadCues fromFile(QFile &file);
	unsigned int getMappedTile(std::size_t frame_index, unsigned int tile_index) const;
};

#endif // DYNAMIC_PATTERN_LOAD_CUES_H
