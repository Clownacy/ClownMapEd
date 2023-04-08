#ifndef DYNAMIC_PATTERN_LOAD_CUES_H
#define DYNAMIC_PATTERN_LOAD_CUES_H

#include <QFile>
#include <QVector>

#include "data-stream.h"
#include "utilities.h"

struct DynamicPatternLoadCues
{
	struct TileCopy
	{
		int start;
		int length;

		int size_encoded() const;
		int total_segments() const;
		void toDataStream(DataStream &stream) const;
	};

	struct Frame
	{
		QVector<TileCopy> copies;

		int getMappedTile(int tile_index) const;
		int size_encoded() const;
		int total_segments() const;
		void toDataStream(DataStream &stream) const;
	};

	QVector<Frame> frames;

	static DynamicPatternLoadCues fromFile(QFile &file);
	void toDataStream(DataStream &stream) const;
};

#endif // DYNAMIC_PATTERN_LOAD_CUES_H
