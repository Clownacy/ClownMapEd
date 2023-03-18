#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <iostream>

#include <QHBoxLayout>
#include <QMainWindow>
#include <QVBoxLayout>

#include "palette.h"
#include "palette-editor.h"
#include "sprite-editor.h"
#include "sprite-mappings.h"
#include "sprite-piece-picker.h"
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
	void loadTileFile(bool (*decompression_function)(std::istream &src, std::iostream &dst));
	void setStartingPaletteLine(int line);

	Ui::MainWindow *ui;
	Palette palette;
	SpriteMappings sprite_mappings;
	TileManager tile_manager;
	SpriteEditor sprite_editor;
	PaletteEditor palette_editor;
	SpritePiecePicker sprite_piece_picker;
	TileViewer tile_viewer;
	QVBoxLayout vertical_layout;
	QHBoxLayout horizontal_layout;
};

#endif // MAIN_WINDOW_H
