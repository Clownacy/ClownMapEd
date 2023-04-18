#ifndef DYNAMIC_PATTERN_LOAD_CUES_H
#define DYNAMIC_PATTERN_LOAD_CUES_H

#include <QFile>
#include <QVector>

#include "data-stream.h"

struct DynamicPatternLoadCues
{
	struct Frame
	{
		struct Copy
		{
			int start;
			int length;

			int size_encoded() const;
			int total_segments() const;
			void toDataStream(DataStream &stream) const;
		};

		QVector<Copy> copies;

		int getMappedTile(int tile_index) const;
		int size_encoded() const;
		int total_segments() const;
		void toDataStream(DataStream &stream) const;
	};

	// TODO: Replace explicit constuctors with a method!
	DynamicPatternLoadCues() = default;
	DynamicPatternLoadCues(QFile &file);

	QVector<Frame> frames;

	void toDataStream(DataStream &stream) const;
};

#endif // DYNAMIC_PATTERN_LOAD_CUES_H
