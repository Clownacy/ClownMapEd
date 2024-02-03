#ifndef DYNAMIC_PATTERN_LOAD_CUES_H
#define DYNAMIC_PATTERN_LOAD_CUES_H

#include <QFile>
#include <QTextStream>

#include "libsonassmd/dynamic-pattern-load-cues.h"

typedef libsonassmd::DynamicPatternLoadCues DynamicPatternLoadCues;

void toQTextStream(const DynamicPatternLoadCues &dplc, QTextStream &stream, DynamicPatternLoadCues::Format format);

#endif // DYNAMIC_PATTERN_LOAD_CUES_H
