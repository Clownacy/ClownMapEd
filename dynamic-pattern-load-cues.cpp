#include "dynamic-pattern-load-cues.h"

#include <QRandomGenerator>

#include "data-stream.h"
#include "utilities.h"

DynamicPatternLoadCues::DynamicPatternLoadCues(QFile &file, const Format format)
{
	DataStream stream(&file);
	stream.setByteOrder(DataStream::BigEndian);

	uint earliest_frame = stream.read<quint16>();
	uint total_frames = 1;

	for (uint position = sizeof(quint16); position < earliest_frame; position += sizeof(quint16))
	{
		const uint frame_offset = stream.read<quint16>();

		if (format == Format::SONIC_2_AND_3_AND_KNUCKLES_AND_CD && (frame_offset & 1) != 0)
			break;

		++total_frames;

		if (earliest_frame > frame_offset)
			earliest_frame = frame_offset;
	}

	frames.resize(total_frames);

	for (uint current_frame = 0; current_frame < total_frames; ++current_frame)
	{
		Frame &frame = frames[current_frame];

		file.seek(current_frame * 2);
		file.seek(stream.read<quint16>());

		const uint total_copies = format == Format::SONIC_2_AND_3_AND_KNUCKLES_AND_CD ? stream.read<quint16>() :  stream.read<quint8>();

		frame.copies.resize(total_copies);

		for (uint current_copy = 0; current_copy < total_copies; ++current_copy)
		{
			const uint word = stream.read<quint16>();
			const int total_tiles = (word >> 4 * 3) + 1;
			const int tile_index = word & 0xFFF;

			frame.copies[current_copy] = Frame::Copy{tile_index, total_tiles};
		}
	}
}

void DynamicPatternLoadCues::toQTextStream(QTextStream &stream, const Format format) const
{
	stream << QStringLiteral("; --------------------------------------------------------------------------------\n"
	                         "; Dynamic Pattern Loading Cues - output from ClownMapEd - %1 format\n"
	                         "; --------------------------------------------------------------------------------\n\n"
	                        ).arg(format == Format::SONIC_1 ? QStringLiteral("Sonic 1") : format == Format::SONIC_2_AND_3_AND_KNUCKLES_AND_CD ? QStringLiteral("Sonic 2/3&K/CD") : QStringLiteral("MapMacros"));

	const QString table_label = format == Format::MAPMACROS ? ".offsets" : "CME_" + Utilities::IntegerToZeroPaddedHexQString(QRandomGenerator::global()->generate());

	stream << table_label << ":";

	if (format == DynamicPatternLoadCues::Format::MAPMACROS)
		stream << "\tmappingsTable";

	stream << "\n";

	for (const auto &frame : qAsConst(frames))
	{
		const QString frame_label = format == Format::MAPMACROS ? ".frame" + QString::number(&frame - frames.data()) : table_label + "_" + QString::number(&frame - frames.data(), 0x10).toUpper();

		if (format == Format::MAPMACROS)
			stream << "\tmappingsTableEntry.w\t" << frame_label << "\n";
		else
			stream << "\tdc.w\t" << frame_label << "-" << table_label << "\n";
	}

	stream << "\n";

	for (const auto &frame : qAsConst(frames))
	{
		const QString frame_label = format == Format::MAPMACROS ? ".frame" + QString::number(&frame - frames.data()) : table_label + "_" + QString::number(&frame - frames.data(), 0x10).toUpper();
		stream << frame_label << ":";

		if (format == DynamicPatternLoadCues::Format::MAPMACROS)
			stream << "\tdplcHeader";

		stream << "\n";
		frame.toQTextStream(stream, format);

		if (format == DynamicPatternLoadCues::Format::MAPMACROS)
			stream << frame_label << "_End\n";

		stream << "\n";
	}

	stream << "\teven\n";
}

int DynamicPatternLoadCues::Frame::getMappedTile(const int tile_index) const
{
	int base_tile = 0;

	for (auto &copy : copies)
	{
		if (tile_index < base_tile + copy.length)
			return copy.start + (tile_index - base_tile);

		base_tile += copy.length;
	}

	return -1;
}

int DynamicPatternLoadCues::Frame::size_encoded() const
{
	int size = 0;
	for (const auto &copy : copies)
		size += copy.size_encoded();
	return size;
}

int DynamicPatternLoadCues::Frame::total_segments() const
{
	int segments = 0;
	for (const auto &copy : copies)
		segments += copy.total_segments();
	return segments;
}

void DynamicPatternLoadCues::Frame::toQTextStream(QTextStream &stream, const Format format) const
{
	if (format != Format::MAPMACROS)
		stream << "\tdc." << (format == Format::SONIC_1 ? 'b' : 'w') << '\t' << total_segments() << '\n';

	for (const auto &copy : copies)
		copy.toQTextStream(stream, format);
}

int DynamicPatternLoadCues::Frame::Copy::size_encoded() const
{
	return sizeof(quint16) * total_segments();
}

int DynamicPatternLoadCues::Frame::Copy::total_segments() const
{
	return Utilities::DivideCeiling(length, 0x10);
}

void DynamicPatternLoadCues::Frame::Copy::toQTextStream(QTextStream &stream, const Format format) const
{
	// TODO: Sanity checks (overflow).
	for (int i = 0; i < total_segments(); ++i)
	{
		const int segment_start = start + 0x10 * i;
		const int segment_length = qMin(0x10, length - 0x10 * i);

		if (format == Format::MAPMACROS)
			stream << "\tdplcEntry\t" << segment_length << ", " << segment_start << "\n";
		else
			stream << "\tdc.w\t$" << Utilities::IntegerToZeroPaddedHexQString(static_cast<quint16>((segment_length - 1) << 12 | (segment_start & 0xFFF))) << '\n';
	}
}
