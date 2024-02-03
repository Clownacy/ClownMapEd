#ifndef DYNAMIC_PATTERN_LOAD_CUES_H
#define DYNAMIC_PATTERN_LOAD_CUES_H

#include <QFile>
#include <QTextStream>

#include "libsonassmd/dynamic-pattern-load-cues.h"

struct DynamicPatternLoadCues : libmdsonass::DynamicPatternLoadCues
{
	struct Frame : libmdsonass::DynamicPatternLoadCues::Frame
	{
		struct Copy : libmdsonass::DynamicPatternLoadCues::Frame::Copy
		{
			void toQTextStream(QTextStream &stream, Format format) const;
		};

		void toQTextStream(QTextStream &stream, Format format) const;
	};

	using libmdsonass::DynamicPatternLoadCues::DynamicPatternLoadCues;

	void toQTextStream(QTextStream &stream, Format format) const;
};

#endif // DYNAMIC_PATTERN_LOAD_CUES_H
