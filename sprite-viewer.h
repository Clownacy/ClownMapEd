#ifndef SPRITE_VIEWER_H
#define SPRITE_VIEWER_H

#include <QColor>
#include <QWidget>

#include "signal-wrapper.h"
#include "sprite-mappings.h"
#include "tile-manager.h"

class SpriteViewer : public QWidget
{
	Q_OBJECT

public:
    SpriteViewer(const TileManager &tile_manager, const SignalWrapper<SpriteMappings> &sprite_mappings);

	uint selected_sprite_index() const
	{
		return m_selected_sprite_index;
	}

	int selected_piece_index() const
	{
		return m_selected_piece_index;
	}

	int starting_palette_line() const
	{
		return m_starting_palette_line;
	}

public slots:
	void setSelectedSprite(uint sprite_index);
	void setSelectedPiece(int piece_index);
	void setBackgroundColour(const QColor &colour);

	void setStartingPaletteLine(const int palette_line)
	{
		m_starting_palette_line = palette_line;
		update();
	}

	void setHideDuplicateTiles(const bool enabled)
	{
		m_hide_duplicate_tiles = enabled;
		update();
	}

signals:
	void selectedSpriteChanged();

protected:
	void paintEvent(QPaintEvent *event) override;

private:
	uint m_selected_sprite_index = -1;
	int m_selected_piece_index = -1;
	int m_starting_palette_line = 0;
	bool m_hide_duplicate_tiles = false;
	const SpriteMappings &sprite_mappings;
	const TileManager &tile_manager;
};

#endif // SPRITE_VIEWER_H
