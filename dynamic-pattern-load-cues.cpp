#include "dynamic-pattern-load-cues.h"

#include <sstream>

#include <QRandomGenerator>

#include "data-stream.h"

void DynamicPatternLoadCues::toQTextStream(QTextStream &stream, const Format format) const
{
	stream << QStringLiteral("; --------------------------------------------------------------------------------\n"
	                         "; Dynamic Pattern Loading Cues - output from ClownMapEd - %1 format\n"
	                         "; --------------------------------------------------------------------------------\n\n"
	                        ).arg(format == Format::SONIC_1 ? QStringLiteral("Sonic 1") : format == Format::SONIC_2_AND_3_AND_KNUCKLES_AND_CD ? QStringLiteral("Sonic 2/3&K/CD") : QStringLiteral("MapMacros"));

	std::stringstream string_stream;
	toStream(string_stream, format);
	stream << string_stream.str().c_str();
}

void DynamicPatternLoadCues::Frame::toQTextStream(QTextStream &stream, const Format format) const
{
	std::stringstream string_stream;
	toStream(string_stream, format);
	stream << string_stream.str().c_str();
}

void DynamicPatternLoadCues::Frame::Copy::toQTextStream(QTextStream &stream, const Format format) const
{
	std::stringstream string_stream;
	toStream(string_stream, format);
	stream << string_stream.str().c_str();
}
