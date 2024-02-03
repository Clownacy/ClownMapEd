#ifndef SPRITE_MAPPINGS_H
#define SPRITE_MAPPINGS_H

#include <QFile>
#include <QTextStream>
#include <QVector>

#include "libsonassmd/sprite-mappings.h"

#include "dynamic-pattern-load-cues.h"
#include "sprite-frame.h"

typedef libsonassmd::SpriteMappings SpriteMappings;

void toQTextStream(const SpriteMappings &mappings, QTextStream &stream, SpritePiece::Format format);

#endif // SPRITE_MAPPINGS_H
