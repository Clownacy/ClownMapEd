#include "sprite-mappings.h"

#include <climits>

#include "data-stream.h"

SpriteMappings SpriteMappings::fromFile(QFile &file)
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

	SpriteMappings mappings;

	mappings.frames.resize(total_frames);

	for (unsigned int current_frame = 0; current_frame < total_frames; ++current_frame)
	{
		file.seek(current_frame * 2);
		file.seek(stream.read<quint16>());

		mappings.frames[current_frame] = SpriteFrame::fromDataStream(stream);
	}

	return mappings;
}
