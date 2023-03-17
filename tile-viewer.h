#ifndef TILE_VIEWER_H
#define TILE_VIEWER_H

#include <QAbstractButton>
#include <QColor>
#include <QObject>

#include "tile-manager.h"

class TileViewer : public QAbstractButton
{
	Q_OBJECT

public:
	TileViewer(const TileManager &tile_manager);

	void setBackgroundColour(const QColor &colour);

public slots:
	void refresh();

private:
	void deleteButtons();
	void paintEvent(QPaintEvent *event) override;

	const TileManager &tile_manager;
	QColor background_colour;
};

#endif // TILE_VIEWER_H
