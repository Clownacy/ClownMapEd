#ifndef TILE_VIEWER_H
#define TILE_VIEWER_H

#include <cstddef>

#include <QObject>
#include <QPushButton>
#include <QWidget>

#include "flowlayout.h"
#include "tile-pixmaps.h"

class TileViewer : public QWidget
{
	Q_OBJECT

public:
	TileViewer(const TilePixmaps &tile_pixmaps);

	void setBackgroundColour(const QColor &colour);

public slots:
	void refresh();

private:
	void deleteButtons();

	const TilePixmaps &tile_pixmaps;
	FlowLayout flow_layout;
};

#endif // TILEVIEWER_H
