#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QHBoxLayout>
#include <QMainWindow>
#include <QVBoxLayout>

#include "palette.h"
#include "palette-editor.h"
#include "sprite-mappings.h"
#include "sprite-viewer.h"
#include "tile-pixmaps.h"
#include "tile-viewer.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();
	void keyPressEvent(QKeyEvent *event) override;

private:
	Ui::MainWindow *ui;
	Palette palette;
	QVector<unsigned char> tiles_bytes;
	SpriteMappings sprite_mappings;
	TilePixmaps tile_pixmaps;
	SpriteViewer sprite_viewer;
	PaletteEditor palette_editor;
	TileViewer tile_viewer;
	QVBoxLayout vertical_box;
	QHBoxLayout horizontal_box;
};

#endif // MAIN_WINDOW_H
