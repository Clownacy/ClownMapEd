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

	void setBackgroundColour(const QColor &colour);

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
};

#endif // TILE_VIEWER_H
