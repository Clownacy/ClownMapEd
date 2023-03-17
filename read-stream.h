#ifndef READ_STREAM_H
#define READ_STREAM_H

#include <QDataStream>

template<typename T>
static inline T Read(QDataStream &in_stream)
{
	T value;
	in_stream >> value;
	return value;
}

#endif // READ_STREAM_H
