#ifndef DYNAMIC_PATTERN_LOAD_CUES_H
#define DYNAMIC_PATTERN_LOAD_CUES_H

#include <QFile>
#include <QTextStream>

#include "libsonassmd/dynamic-pattern-load-cues.h"

struct DynamicPatternLoadCues : libsonassmd::DynamicPatternLoadCues
{
	struct Frame : libsonassmd::DynamicPatternLoadCues::Frame
	{
		struct Copy : libsonassmd::DynamicPatternLoadCues::Frame::Copy
		{
			void toQTextStream(QTextStream &stream, Format format) const;
		};

		void toQTextStream(QTextStream &stream, Format format) const;
	};

	using libsonassmd::DynamicPatternLoadCues::DynamicPatternLoadCues;

	void toQTextStream(QTextStream &stream, Format format) const;
};

#endif // DYNAMIC_PATTERN_LOAD_CUES_H
