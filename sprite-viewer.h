#ifndef SPRITE_VIEWER_H
#define SPRITE_VIEWER_H

#include <QPixmap>
#include <QWidget>

#include "sprite-mappings.h"
#include "tile-pixmaps.h"

class SpriteViewer : public QWidget
{
	Q_OBJECT

public:
	SpriteViewer(const TilePixmaps &tile_pixmaps, const SpriteMappings &sprite_mappings);

	void selectNextSprite();
	void selectPreviousSprite();

	void setBackgroundColour(const QColor &colour);

protected:
	void paintEvent(QPaintEvent *event) override;

private:
	unsigned int selected_sprite_index;
	QPixmap pixmap;
	const SpriteMappings &sprite_mappings;
	const TilePixmaps &tile_pixmaps;

	void paintFrame(QPainter &painter, unsigned int frame_index, int x_offset = 0, int y_offset = 0);
};

#endif // SPRITE_VIEWER_H
