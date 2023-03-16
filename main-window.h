#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <vector>

#include <QMainWindow>

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
	std::vector<unsigned char> tiles_bytes;
	SpriteMappings sprite_mappings;
	TilePixmaps tile_pixmaps;
	SpriteViewer sprite_viewer;
	PaletteEditor palette_editor;
	TileViewer tile_viewer;
};

#endif // MAIN_WINDOW_H
