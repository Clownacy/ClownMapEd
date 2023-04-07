#include "dynamic-pattern-load-cues.h"

#include "data-stream.h"

DynamicPatternLoadCues DynamicPatternLoadCues::fromFile(QFile &file)
{
	DataStream stream(&file);
	stream.setByteOrder(DataStream::BigEndian);

	uint earliest_frame = stream.read<quint16>();
	uint total_frames = 1;

	while (file.pos() < earliest_frame)
	{
		const uint frame_offset = stream.read<quint16>();

		if ((frame_offset & 1) != 0)
			break;

		++total_frames;

		if (earliest_frame > frame_offset)
			earliest_frame = frame_offset;
	}

	DynamicPatternLoadCues dplcs;

	dplcs.frames.resize(total_frames);

	for (uint current_frame = 0; current_frame < total_frames; ++current_frame)
	{
		Frame &frame = dplcs.frames[current_frame];

		file.seek(current_frame * 2);
		file.seek(stream.read<quint16>());

		const uint total_copies = stream.read<quint16>();

		frame.copies.resize(total_copies);

		for (uint current_copy = 0; current_copy < total_copies; ++current_copy)
		{
			const uint word = stream.read<quint16>();
			const int total_tiles = (word >> 4 * 3) + 1;
			const int tile_index = word & 0xFFF;

			frame.copies[current_copy] = TileCopy{tile_index, total_tiles};
		}
	}

	return dplcs;
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
