#include "sprite-mappings.h"

#include <sstream>

void toQTextStream(const SpriteMappings &mappings, QTextStream &stream, const SpritePiece::Format format)
{
	// TODO: This code is duplicated in the DPLC code. Can this be made into a common function?
	stream << QStringLiteral("; --------------------------------------------------------------------------------\n"
	                         "; Sprite mappings - output from ClownMapEd - %1 format\n"
	                         "; --------------------------------------------------------------------------------\n\n"
	                        ).arg(format == SpritePiece::Format::SONIC_1 ? QStringLiteral("Sonic 1/CD") : format == SpritePiece::Format::SONIC_2 ? QStringLiteral("Sonic 2") : format == SpritePiece::Format::SONIC_3_AND_KNUCKLES ? QStringLiteral("Sonic 3 & Knuckles") : QStringLiteral("MapMacros"));

	// TODO: Don't do this.
	std::stringstream string_stream;
	mappings.toStream(string_stream, format);
	stream << string_stream.str().c_str();
}
