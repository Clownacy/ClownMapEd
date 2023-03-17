#include "sprite-mappings.h"

#include <climits>

#include <QDataStream>
#include <QFile>

#include "read-stream.h"

SpriteMappings::SpriteMappings()
{

}

bool SpriteMappings::loadFromFile(const QString &file_path)
{
	QFile file(file_path);
	if (!file.open(QFile::ReadOnly))
		return false;

	QDataStream in_stream(&file);
	in_stream.setByteOrder(QDataStream::BigEndian);

	unsigned int earliest_frame = Read<quint16>(in_stream);
	unsigned int total_frames = 1;

	while (file.pos() < earliest_frame)
	{
		const unsigned int frame_offset = Read<quint16>(in_stream);

		if ((frame_offset & 1) != 0)
			break;

		++total_frames;

		if (earliest_frame > frame_offset)
			earliest_frame = frame_offset;
	}

	m_frames.resize(total_frames);
	m_frames.squeeze();

	for (unsigned int current_frame = 0; current_frame < total_frames; ++current_frame)
	{
		file.seek(current_frame * 2);
		file.seek(Read<quint16>(in_stream));

		m_frames[current_frame].fromDataStream(in_stream);
	}

	return true;
}
