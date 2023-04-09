#ifndef GUARD_H
#define GUARD_H

#include <QObject>

class SignalWrapperBase : public QObject
{
	Q_OBJECT

signals:
	void modified();
};

template<typename T>
class SignalWrapper : public SignalWrapperBase
{
public:
	T* operator->()
	{
		return &data;
	}
	const T* operator->() const
	{
		return &data;
	}
	T& operator*()
	{
		return data;
	}
	const T& operator*() const
	{
		return data;
	}

	void modify(const std::function<void(T &data)> &callback)
	{
		callback(data);

		emit modified();
	}

private:
	T data;
};

#endif // GUARD_H
