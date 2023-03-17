#ifndef DATA_STREAM_H
#define DATA_STREAM_H

#include <QDataStream>

class DataStream : public QDataStream
{
public:
	using QDataStream::QDataStream;

	template<typename T>
	T read()
	{
		T value;
		*this >> value;
		return value;
	}
};

#endif // DATASTREAM_H
