#include "dynamic-pattern-load-cues.h"

#include <sstream>

void toQTextStream(const DynamicPatternLoadCues &dplc, QTextStream &stream, const DynamicPatternLoadCues::Format format)
{
	stream << QStringLiteral("; --------------------------------------------------------------------------------\n"
	                         "; Dynamic Pattern Loading Cues - output from ClownMapEd - %1 format\n"
	                         "; --------------------------------------------------------------------------------\n\n"
	                        ).arg(format == DynamicPatternLoadCues::Format::SONIC_1 ? QStringLiteral("Sonic 1") : format == DynamicPatternLoadCues::Format::SONIC_2_AND_3_AND_KNUCKLES_AND_CD ? QStringLiteral("Sonic 2/3&K/CD") : QStringLiteral("MapMacros"));

	// TODO: Don't do this.
	std::stringstream string_stream;
	dplc.toStream(string_stream, format);
	stream << string_stream.str().c_str();
}
