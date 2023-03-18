#ifndef TILE_VIEWER_H
#define TILE_VIEWER_H

#include <QColor>
#include <QObject>
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
	}

	void setPaletteLine(const int palette_line)
	{
		this->palette_line = palette_line;
		update();
	}

signals:
	void tileSelected(int tile);

private:
	void deleteButtons();
	void getGridDimensions(int &width, int &height);
	void paintEvent(QPaintEvent *event) override;
	void mousePressEvent(QMouseEvent *event) override;

	const TileManager &tile_manager;
	QColor background_colour;
	int selection_start = 0;
	int selection_end = 0;
	int palette_line = 0;
};

#endif // TILE_VIEWER_H
