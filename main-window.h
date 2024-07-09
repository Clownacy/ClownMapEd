#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QHBoxLayout>
#include <QMainWindow>
#include <QVBoxLayout>

#include "about.h"
#include "licences.h"
#include "palette.h"
#include "palette-editor.h"
#include "signal-wrapper.h"
#include "sprite-mappings.h"
#include "sprite-piece-picker.h"
#include "sprite-viewer.h"
#include "tile-manager.h"
#include "tile-viewer.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

// Make libsonassmd::Game compatible with QSettings.
namespace libsonassmd {
	Q_NAMESPACE
	Q_ENUM_NS(Game)
}
Q_DECLARE_METATYPE(libsonassmd::Game)

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();

private:
	Ui::MainWindow *ui;
	SignalWrapper<Palette> palette;
	SignalWrapper<SpriteMappings> sprite_mappings;
	TileManager tile_manager;
	SpriteViewer sprite_viewer;
	PaletteEditor palette_editor;
	SpritePiecePicker sprite_piece_picker;
	TileViewer tile_viewer;
	QVBoxLayout vertical_layout;
	QHBoxLayout horizontal_layout;
	About about = About(this);
	Licence licences = Licence(this);
};

#endif // MAIN_WINDOW_H
