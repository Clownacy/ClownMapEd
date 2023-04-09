#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QHBoxLayout>
#include <QMainWindow>
#include <QVBoxLayout>

#include "palette.h"
#include "palette-editor.h"
#include "sprite-mappings-manager.h"
#include "sprite-piece-picker.h"
#include "sprite-viewer.h"
#include "tile-manager.h"
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

private:
	Ui::MainWindow *ui;
	PaletteManager palette_manager;
	SpriteMappingsManager sprite_mappings_manager;
	TileManager tile_manager;
	SpriteViewer sprite_viewer;
	PaletteEditor palette_editor;
	SpritePiecePicker sprite_piece_picker;
	TileViewer tile_viewer;
	QVBoxLayout vertical_layout;
	QHBoxLayout horizontal_layout;
};

#endif // MAIN_WINDOW_H
