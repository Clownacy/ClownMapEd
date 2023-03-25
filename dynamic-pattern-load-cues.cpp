#include "dynamic-pattern-load-cues.h"

#include "data-stream.h"

DynamicPatternLoadCues DynamicPatternLoadCues::fromFile(QFile &file)
{
	DataStream stream(&file);
	stream.setByteOrder(DataStream::BigEndian);

	unsigned int earliest_frame = stream.read<quint16>();
	unsigned int total_frames = 1;

	while (file.pos() < earliest_frame)
	{
		const unsigned int frame_offset = stream.read<quint16>();

		if ((frame_offset & 1) != 0)
			break;

		++total_frames;

		if (earliest_frame > frame_offset)
			earliest_frame = frame_offset;
	}

	DynamicPatternLoadCues dplcs;

	dplcs.frames.resize(total_frames);

	for (unsigned int current_frame = 0; current_frame < total_frames; ++current_frame)
	{
		QVector<TileCopy> &frame = dplcs.frames[current_frame];

		file.seek(current_frame * 2);
		file.seek(stream.read<quint16>());

		const unsigned int total_copies = stream.read<quint16>();

		frame.resize(total_copies);

		for (unsigned int current_copy = 0; current_copy < total_copies; ++current_copy)
		{
			const unsigned int word = stream.read<quint16>();
			const unsigned int total_tiles = (word >> 4 * 3) + 1;
			const unsigned int tile_index = word & 0xFFF;

			frame[current_copy] = TileCopy{tile_index, total_tiles};
		}
	}

	return dplcs;
}

unsigned int DynamicPatternLoadCues::getMappedTile(const std::size_t frame_index, const unsigned int tile_index) const
{
	unsigned int base_tile = 0;

	for (auto &copy : frames[frame_index])
	{
		if (tile_index < base_tile + copy.length)
			return copy.start + (tile_index - base_tile);

		base_tile += copy.length;
	}

	return static_cast<unsigned int>(-1);
}
