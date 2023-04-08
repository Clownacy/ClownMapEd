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

	template<typename T>
	void write(const T &value)
	{
		*this << value;
	}
};

#endif // DATASTREAM_H
