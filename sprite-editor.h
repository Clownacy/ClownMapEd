#ifndef SPRITE_EDITOR_H
#define SPRITE_EDITOR_H

#include <QWidget>

#include "sprite-mappings-manager.h"
#include "tile-manager.h"

class SpriteEditor : public QWidget
{
	Q_OBJECT

public:
	SpriteEditor(const TileManager &tile_manager, const SpriteMappingsManager &sprite_mappings_manager);

	int selected_sprite_index() const
	{
		return m_selected_sprite_index;
	}

public slots:
	void setSelectedSprite(int sprite_index);

	void setBackgroundColour(const QColor &colour);

	void setStartingPaletteLine(const int palette_line)
	{
		starting_palette_line = palette_line;
		update();
	}

signals:
	void selectedSpriteChanged(int selected_sprite_index);

protected:
	void paintEvent(QPaintEvent *event) override;

private:
	int m_selected_sprite_index = 0;
	int starting_palette_line = 0;
	const SpriteMappings &sprite_mappings;
	const TileManager &tile_manager;
};

#endif // SPRITE_EDITOR_H
