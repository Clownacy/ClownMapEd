#ifndef SPRITEMAPPINGSMANAGER_H
#define SPRITEMAPPINGSMANAGER_H

#include <QObject>

#include "sprite-mappings.h"

class SpriteMappingsManager : public QObject
{
	Q_OBJECT

	class Lock;

public:
	const SpriteMappings& sprite_mappings() const
	{
		return m_sprite_mappings;
	}

	Lock lock()
	{
		return Lock(*this);
	}

signals:
	void mappingsModified();

private:
	SpriteMappings m_sprite_mappings;

	class Lock : public QObject
	{
	public:
		Lock(SpriteMappingsManager &parent)
			: parent(parent)
		{
			connect(this, &Lock::destroyed, &parent, &SpriteMappingsManager::mappingsModified);
		}

		SpriteMappings* operator->()
		{
			return &parent.m_sprite_mappings;
		}

		void operator()(const SpriteMappings &mappings)
		{
			parent.m_sprite_mappings = mappings;
		}

	private:
		SpriteMappingsManager &parent;
	};
};

#endif // SPRITEMAPPINGSMANAGER_H
