#ifndef SPRITE_VIEWER_H
#define SPRITE_VIEWER_H

#include <optional>

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

	std::optional<int> selectedSpriteIndex() const
	{
		return selected_sprite_index;
	}

	std::optional<int> selectedPieceIndex() const
	{
		return selected_piece_index;
	}

	int startingPaletteLine() const
	{
		return starting_palette_line;
	}

public slots:
	void setSelectedSprite(std::optional<int> sprite_index);
	void setSelectedPiece(std::optional<int> piece_index);
	void selectNextPiece();
	void selectPreviousPiece();
	void setBackgroundColour(const QColor &colour);

	void setStartingPaletteLine(const int palette_line)
	{
		starting_palette_line = palette_line;
		update();
	}

	void setHideDuplicateTiles(const bool enabled)
	{
		hide_duplicate_tiles = enabled;
		update();
	}

signals:
	void selectedSpriteChanged();

protected:
	void paintEvent(QPaintEvent *event) override;

private:
	std::optional<int> selected_sprite_index;
	std::optional<int> selected_piece_index;
	int starting_palette_line = 0;
	bool hide_duplicate_tiles = false;
	const SpriteMappings &sprite_mappings;
	const TileManager &tile_manager;

	int totalPiecesInSelectedSprite() const
	{
		return static_cast<int>(sprite_mappings.frames[selected_sprite_index.value()].pieces.size());
	}
};

#endif // SPRITE_VIEWER_H
