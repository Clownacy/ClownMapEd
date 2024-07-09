#ifndef TILE_VIEWER_H
#define TILE_VIEWER_H

#include <functional>

#include <QColor>
#include <QObject>
#include <QTimer>
#include <QVector>
#include <QWidget>

#include "tile-manager.h"

class TileViewer : public QWidget
{
	Q_OBJECT

public:
	TileViewer(const TileManager &tile_manager);

	void setBackgroundColour(const QColor &colour)
	{
		background_colour = colour;
		update();
	}

	int paletteLine() const
	{
		return palette_line;
	}

	void setPaletteLine(const int palette_line)
	{
		this->palette_line = palette_line;
		update();
	}

	void clearSelection()
	{
		selected.fill(false);
	}

	const QVector<bool>& selection() const
	{
		return selected;
	}

	void setSelection(bool scroll_to_selection, const std::function<void(QVector<bool> &selection)> &callback);

	int scroll() const
	{
		return m_scroll;
	}

	void setScroll(const int scroll)
	{
		m_scroll = qBound(0, scroll, tile_manager.total_tiles() - 1);
		update();
	}

	void getGridDimensions(int &width, int &height);

signals:
	void tileSelected(int tile);

private:
	void deleteButtons();
	void paintEvent(QPaintEvent *event) override;
	void mousePressEvent(QMouseEvent *event) override;

	const TileManager &tile_manager;
	QColor background_colour;
	QTimer timer;
	QVector<bool> selected;
	bool selection_flip_flop = false;
	int palette_line = 0;
	int m_scroll = 0;
};

#endif // TILE_VIEWER_H
