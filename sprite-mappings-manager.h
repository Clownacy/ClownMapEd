#ifndef SPRITEMAPPINGSMANAGER_H
#define SPRITEMAPPINGSMANAGER_H

#include <functional>

#include <QObject>

#include "sprite-mappings.h"

class SpriteMappingsManager : public QObject
{
	Q_OBJECT

public:
	const SpriteMappings& sprite_mappings() const
	{
		return m_sprite_mappings;
	}

	void modifySpriteMappings(const std::function<void(SpriteMappings&)> &callback)
	{
		callback(m_sprite_mappings);

		emit mappingsModified();
	}

signals:
	void mappingsModified();

private:
	SpriteMappings m_sprite_mappings;
};

#endif // SPRITEMAPPINGSMANAGER_H
