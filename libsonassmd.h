#ifndef LIBSONASSMD_WRAPPER_H
#define LIBSONASSMD_WRAPPER_H

#include <QString>

#include "libsonassmd/dynamic-pattern-load-cues.h"
#include "libsonassmd/sprite-mappings.h"
#include "libsonassmd/tiles.h"

#define LIBSONASSMD_WRAPPER(TYPE) \
class TYPE : public libsonassmd::TYPE \
{ \
public: \
	using libsonassmd::TYPE::TYPE; \
	using libsonassmd::TYPE::fromFile; \
	using libsonassmd::TYPE::toFile; \
	TYPE(const QString &file_path, const Format format) : TYPE(file_path.toStdString(), format) {} \
	void fromFile(const QString &file_path, const Format format) {fromFile(file_path.toStdString(), format);} \
	void toFile(const QString &file_path, const Format format) const {toFile(file_path.toStdString(), format);} \
}

LIBSONASSMD_WRAPPER(DynamicPatternLoadCues);
LIBSONASSMD_WRAPPER(SpriteMappings);
LIBSONASSMD_WRAPPER(Tiles);

#endif // LIBSONASSMD_WRAPPER_H
