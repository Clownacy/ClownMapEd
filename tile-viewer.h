#ifndef TILE_VIEWER_H
#define TILE_VIEWER_H

#include <QAbstractButton>
#include <QColor>
#include <QObject>

#include "tile-pixmaps.h"

class TileViewer : public QAbstractButton
{
	Q_OBJECT

public:
	TileViewer(const TilePixmaps &tile_pixmaps);

	void setBackgroundColour(const QColor &colour);

public slots:
	void refresh();

private:
	void deleteButtons();
	void paintEvent(QPaintEvent *event) override;

	const TilePixmaps &tile_pixmaps;
	QColor background_colour;
};

#endif // TILEVIEWER_H
