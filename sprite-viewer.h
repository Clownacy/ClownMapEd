#ifndef SPRITE_VIEWER_H
#define SPRITE_VIEWER_H

#include <QWidget>

#include "sprite-mappings.h"
#include "tile-manager.h"

class SpriteViewer : public QWidget
{
	Q_OBJECT

public:
	SpriteViewer(const TileManager &tile_manager, const SpriteMappings &sprite_mappings);

	void selectNextSprite();
	void selectPreviousSprite();

	void setBackgroundColour(const QColor &colour);

protected:
	void paintEvent(QPaintEvent *event) override;

private:
	int selected_sprite_index = 0;
	const SpriteMappings &sprite_mappings;
	const TileManager &tile_manager;
};

#endif // SPRITE_VIEWER_H
