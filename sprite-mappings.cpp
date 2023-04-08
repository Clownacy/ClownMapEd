#include "sprite-mappings.h"

#include "data-stream.h"

SpriteMappings SpriteMappings::fromFile(QFile &file)
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

	SpriteMappings mappings;

	mappings.frames.resize(total_frames);

	for (uint current_frame = 0; current_frame < total_frames; ++current_frame)
	{
		file.seek(current_frame * 2);
		file.seek(stream.read<quint16>());

		mappings.frames[current_frame] = SpriteFrame::fromDataStream(stream);
	}

	return mappings;
}

void SpriteMappings::toDataStream(DataStream &stream) const
{
	const auto original_byte_order = stream.byteOrder();
	stream.setByteOrder(DataStream::BigEndian);

	int current_offset = frames.size() * sizeof(quint16);
	for (const auto &frame : frames)
	{
		stream.write<quint16>(current_offset);
		current_offset += frame.size_encoded();
	}

	for (const auto &frame : frames)
		frame.toDataStream(stream);

	stream.setByteOrder(original_byte_order);
}

bool SpriteMappings::applyDPLCs(const DynamicPatternLoadCues &dplcs)
{
	if (frames.size() == dplcs.frames.size())
	{
		for (int frame_index = 0; frame_index < frames.size(); ++frame_index)
		{
			auto &dplc_frame = dplcs.frames[frame_index];

			for (auto &piece : frames[frame_index].pieces)
			{
				const int base_mapped_tile = dplc_frame.getMappedTile(piece.tile_index);

				for (int i = 0; i < piece.width * piece.height; ++i)
				{
					const int mapped_tile = dplc_frame.getMappedTile(piece.tile_index + i);

					if (mapped_tile == -1 || mapped_tile != base_mapped_tile + i)
						return false;
				}

				piece.tile_index = base_mapped_tile;
			}
		}
	}

	return true;
}

DynamicPatternLoadCues SpriteMappings::removeDPLCs()
{
	// Determine the total number of tiles.
	int total_tiles = 0;
	for (auto &frame : frames)
		for (auto &piece : frame.pieces)
			total_tiles = qMax(total_tiles, piece.tile_index + piece.width * piece.height);

	// I suppose a hash map would work here too.
	QVector<int> tile_indices;
	tile_indices.resize(total_tiles);

	DynamicPatternLoadCues dplcs;
	dplcs.frames.reserve(frames.size());

	// Produce DPLC frames and modify the mappings frames to be relative to them.
	for (auto &frame : frames)
	{
		// Mark which tiles are mapped to which DPLC tiles.
		tile_indices.fill(false);

		for (auto &piece : frame.pieces)
			for (int i = 0; i < piece.width * piece.height; ++i)
				tile_indices[piece.tile_index + i] = true;

		// Use the marked tiles to determine the DPLC data for this frame.
		DynamicPatternLoadCues::Frame dplc_frame;

		for (int search_start = 0;;)
		{
			const int dplc_start = tile_indices.indexOf(true, search_start);

			if (dplc_start == -1)
				break;

			search_start = tile_indices.indexOf(false, dplc_start);

			if (search_start == -1)
				search_start = total_tiles;

			const int dplc_length = search_start - dplc_start;

			dplc_frame.copies.append({dplc_start, dplc_length});
		}

		dplcs.frames.push_back(dplc_frame);

		// Go back and modify the mappings to be relative to the DPLC data.
		int current_tile_index = 0;
		for (auto &tile_index : tile_indices)
			if (tile_index != false)
				tile_index = current_tile_index++;

		for (auto &piece : frame.pieces)
			piece.tile_index = tile_indices[piece.tile_index];
	}

	return dplcs;
}
