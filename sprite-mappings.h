#ifndef SPRITE_MAPPINGS_H
#define SPRITE_MAPPINGS_H

#include <cstddef>

#include <QPainter>
#include <QString>
#include <QVector>

#include "sprite-frame.h"

class SpriteMappings
{
public:
	SpriteMappings();

	bool loadFromFile(const QString &file_path);

	const QVector<SpriteFrame>& frames() const
	{
		return m_frames;
	}

private:
	QVector<SpriteFrame> m_frames;
};

#endif // SPRITE_MAPPINGS_H
