#include "sprite-mappings.h"

#include <climits>

#include <QDataStream>
#include <QFile>

template<typename T>
static inline T Read(QDataStream &in_stream)
{
	T value;
	in_stream >> value;
	return value;
}

SpriteMappings::SpriteMappings()
{

}

void SpriteMappings::loadFromFile(const QString &file_path)
{
	QFile file(file_path);
	if (!file.open(QFile::ReadOnly))
		return;

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

	frames.resize(total_frames);

	unsigned int total_pieces = 0;

	for (unsigned int current_frame = 0; current_frame < total_frames; ++current_frame)
	{
		file.seek(current_frame * 2);
		file.seek(Read<quint16>(in_stream));
		total_pieces += Read<quint16>(in_stream);
	}

	pieces.resize(total_pieces);

	unsigned int piece_index = 0;

	for (unsigned int current_frame = 0; current_frame < total_frames; ++current_frame)
	{
		file.seek(current_frame * 2);
		file.seek(Read<quint16>(in_stream));

		Frame &frame = frames[current_frame];

		frame.total_pieces = Read<quint16>(in_stream);
		frame.pieces = &pieces[piece_index];

		frame.x1 = INT_MAX;
		frame.x2 = INT_MIN;
		frame.y1 = INT_MAX;
		frame.y2 = INT_MIN;

		for (unsigned int current_piece = 0; current_piece < frame.total_pieces; ++current_piece)
		{
			Piece &piece = pieces[piece_index];

			piece.y = Read<qint8>(in_stream);
			const unsigned int size = Read<quint8>(in_stream);
			piece.width = ((size >> 2) & 3) + 1;
			piece.height = ((size >> 0) & 3) + 1;
			const unsigned int art_tile = Read<quint16>(in_stream);
			piece.priority = (art_tile & (1 << 15)) != 0;
			piece.palette_line = (art_tile >> 13) & 3;
			piece.y_flip = (art_tile & (1 << 12)) != 0;
			piece.x_flip = (art_tile & (1 << 11)) != 0;
			piece.tile_index = art_tile & 0x7FF;
			Read<quint16>(in_stream); // TODO - 2-player data?
			piece.x = Read<qint16>(in_stream);

			if (frame.x1 > piece.x)
				frame.x1 = piece.x;
			if (frame.x2 < piece.x + piece.width * 8)
				frame.x2 = piece.x + piece.width * 8;
			if (frame.y1 > piece.y)
				frame.y1 = piece.y;
			if (frame.y2 < piece.y + piece.height * 8)
				frame.y2 = piece.y + piece.height * 8;

			++piece_index;
		}

		if (frame.x1 == INT_MAX)
			frame.x1 = 0;
		if (frame.x2 == INT_MIN)
			frame.x2 = 0;
		if (frame.y1 == INT_MAX)
			frame.y1 = 0;
		if (frame.y2 == INT_MIN)
			frame.y2 = 0;
	}
}

void SpriteMappings::paintPiece(QPainter &painter, const TilePixmaps &tile_pixmaps, const Piece &piece, int x_offset, int y_offset) const
{
	const QTransform flip_transform = QTransform::fromScale(piece.x_flip ? -1 : 1, piece.y_flip ? -1 : 1);

	unsigned int tile_index = piece.tile_index;

	for (int tile_x = 0; tile_x < piece.width; ++tile_x)
	{
		const int tile_x_corrected = piece.x_flip ? piece.width - tile_x - 1 : tile_x;

		for (int tile_y = 0; tile_y < piece.height; ++tile_y)
		{
			const int tile_y_corrected = piece.y_flip ? piece.height - tile_y - 1 : tile_y;

			const QRect rect(
				x_offset + piece.x + tile_x_corrected * 8,
				y_offset + piece.y + tile_y_corrected * 8,
				8,
				8
			);

			painter.drawPixmap(rect, tile_pixmaps[tile_index++].transformed(flip_transform), QRectF(0, 0, 8, 8));
		}
	}
}

void SpriteMappings::paintFrame(QPainter &painter, const TilePixmaps &tile_pixmaps, const Frame &frame, int x_offset, int y_offset) const
{
	for (unsigned int current_piece = 0; current_piece < frame.total_pieces; ++current_piece)
		paintPiece(painter, tile_pixmaps, frame.pieces[current_piece], x_offset, y_offset);
}
