#ifndef TILE_VIEWER_H
#define TILE_VIEWER_H

#include <QColor>
#include <QObject>
#include <QTimer>
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

	void setPaletteLine(const int palette_line)
	{
		this->palette_line = palette_line;
		update();
	}

	void setSelection(int start, int end);

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
	bool selection_flip_flop = false;
	int selection_start = 0;
	int selection_end = 0;
	int palette_line = 0;
	int m_scroll = 0;
};

#endif // TILE_VIEWER_H
