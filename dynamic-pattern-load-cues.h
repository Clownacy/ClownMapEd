#ifndef DYNAMIC_PATTERN_LOAD_CUES_H
#define DYNAMIC_PATTERN_LOAD_CUES_H

#include <QFile>
#include <QTextStream>
#include <QVector>

#include "data-stream.h"

struct DynamicPatternLoadCues
{
	enum class Format
	{
		SONIC_1,
		SONIC_2_AND_3_AND_KNUCKLES_AND_CD,
		MAPMACROS
	};

	struct Frame
	{
		struct Copy
		{
			int start;
			int length;

			int size_encoded() const;
			int total_segments() const;
			void toQTextStream(QTextStream &stream, Format format) const;
		};

		QVector<Copy> copies;

		int getMappedTile(int tile_index) const;
		int size_encoded() const;
		int total_segments() const;
		void toQTextStream(QTextStream &stream, Format format) const;
	};

	// TODO: Replace explicit constuctors with a method!
	DynamicPatternLoadCues() = default;
	DynamicPatternLoadCues(QFile &file, Format format);

	QVector<Frame> frames;

	void toQTextStream(QTextStream &stream, Format format) const;
};

#endif // DYNAMIC_PATTERN_LOAD_CUES_H
