#include "main-window.h"
#include "./ui_main-window.h"

#include <climits>
#include <fstream>
#include <functional>

#include <QDataStream>
#include <QFile>
#include <QFileDialog>
#include <QKeyEvent>
#include <QMessageBox>

#include "comper.h"
#include "kosinski.h"
#include "kosplus.h"
#include "nemesis.h"

#include "dynamic-pattern-load-cues.h"

MainWindow::MainWindow(QWidget* const parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
    , tile_manager(palette)
	, sprite_viewer(tile_manager, sprite_mappings)
    , palette_editor(palette)
	, sprite_piece_picker(tile_manager)
    , tile_viewer(tile_manager)
{
	ui->setupUi(this);

	palette.modify(
		[](Palette &palette)
		{
			palette.reset();
		}
	);

	horizontal_layout.addWidget(&sprite_piece_picker);
	horizontal_layout.addWidget(&palette_editor);
	horizontal_layout.addWidget(&sprite_viewer);

	vertical_layout.addLayout(&horizontal_layout);
//	vbox->addsp
	vertical_layout.addWidget(&tile_viewer);

	centralWidget()->setLayout(&vertical_layout);

	horizontal_layout.setMargin(vertical_layout.margin());
	vertical_layout.setMargin(0);

	///////////////////////
	// Background Colour //
	///////////////////////

	const auto update_background_colour = [this]()
	{
		const QColor &background_colour = palette->lines[0].colours[0].toQColor224();

		sprite_viewer.setBackgroundColour(background_colour);
		tile_viewer.setBackgroundColour(background_colour);
	};

	connect(&palette, &SignalWrapper<Palette>::modified, this, update_background_colour);

	update_background_colour();

	///////////////////
	// Misc. Signals //
	///////////////////

	connect(&tile_viewer, &TileViewer::tileSelected, &sprite_piece_picker, &SpritePiecePicker::setSelectedTile);

	const auto tile_viewer_select_sprite = [this]()
	{
		tile_viewer.clearSelection();
		tile_viewer.setSelection(true,
			[this](QVector<bool> &selection)
			{
				const auto do_frame_or_piece = [this](const std::function<void(const SpritePiece &piece)> &callback)
				{
					const int frame_index = sprite_viewer.selected_sprite_index();

					if (frame_index != -1)
					{
						const auto &frames = sprite_mappings->frames;
						const int piece_index = sprite_viewer.selected_piece_index();

						if (piece_index == -1)
						{
							if (frames[frame_index].pieces.size() != 0)
								sprite_piece_picker.setSelectedTile(frames[frame_index].pieces[0].tile_index);

							for (auto &piece : frames[frame_index].pieces)
								callback(piece);
						}
						else
						{
							const auto &piece = frames[frame_index].pieces[piece_index];

							sprite_piece_picker.setSelectedTile(piece.tile_index);

							callback(piece);
						}
					}
				};

				do_frame_or_piece(
					[&selection](const SpritePiece &piece)
					{
						for (int i = 0; i < piece.width * piece.height; ++i)
							if (piece.tile_index + i < selection.size())
								selection[piece.tile_index + i] = true;
					}
				);
			}
		);
	};

	connect(&sprite_viewer, &SpriteViewer::selectedSpriteChanged, this, tile_viewer_select_sprite);
	connect(&sprite_mappings, &SignalWrapper<SpriteMappings>::modified, this, tile_viewer_select_sprite);

	connect(&sprite_piece_picker, &SpritePiecePicker::pieceSelected, this,
		[this](const int width, const int height)
		{
			sprite_mappings.modify(
				[this, width, height](SpriteMappings &mappings)
				{
					if (sprite_viewer.selected_sprite_index() == -1)
					{
						mappings.frames.append(SpriteFrame());
						sprite_viewer.setSelectedSprite(0);
					}

					auto &pieces = mappings.frames[sprite_viewer.selected_sprite_index()].pieces;
					pieces.append(SpritePiece{0, 0, width, height, false, 0, false, false, sprite_piece_picker.selected_tile()});
					sprite_viewer.setSelectedPiece(pieces.size() - 1);

					sprite_piece_picker.setSelectedTile(sprite_piece_picker.selected_tile() + width * height);
				}
			);
		}
	);

	//////////////////////////////////
	// Menubar: File/Load Data File //
	//////////////////////////////////

	const auto load_tile_file = [this](const QString &file_path, bool (* const decompression_function)(std::istream &src, std::iostream &dst))
	{
		if (!file_path.isNull())
		{
			std::ifstream file_stream(file_path.toStdString(), std::ifstream::in | std::ifstream::binary);

			if (!file_stream.is_open())
			{
				QMessageBox::critical(this, "Error", "Failed to load file: file could not be opened for reading.");
				return;
			}

			std::stringstream string_stream(std::stringstream::in | std::stringstream::out | std::stringstream::binary);

			if (!decompression_function(file_stream, string_stream))
			{
				QMessageBox::critical(this, "Error", "Failed to load file: data could not be decompressed file. Your chosen compression format may have been incorrect.");
				return;
			}

			const std::string string = string_stream.str();
			if (!tile_manager.setTiles(string.begin(), string.end()))
			{
				QMessageBox::critical(this, "Error", "Failed to load file: data ends with an incomplete tile. The file might not actually be tile data.");
				return;
			}
		}
	};

	const auto load_uncompressed_tile_file = [load_tile_file](const QString &file_path)
	{
		load_tile_file(file_path, [](std::istream &src, std::iostream &dst)
			{
				dst << src.rdbuf();
				return true;
			}
		);
	};

	const auto load_nemesis_tile_file = [load_tile_file](const QString &file_path)
	{
		load_tile_file(file_path, [](std::istream &src, std::iostream &dst){return nemesis::decode(src, dst);});
	};

	const auto load_kosinski_tile_file = [load_tile_file](const QString &file_path)
	{
		load_tile_file(file_path, kosinski::decode);
	};

	const auto load_moduled_kosinski_tile_file = [load_tile_file](const QString &file_path)
	{
		load_tile_file(file_path, [](std::istream &src, std::iostream &dst){return kosinski::moduled_decode(src, dst);});
	};

	const auto load_kosinski_plus_tile_file = [load_tile_file](const QString &file_path)
	{
		load_tile_file(file_path, kosplus::decode);
	};

	const auto load_moduled_kosinski_plus_tile_file = [load_tile_file](const QString &file_path)
	{
		load_tile_file(file_path, [](std::istream &src, std::iostream &dst){return kosplus::moduled_decode(src, dst);});
	};

	const auto load_comper_tile_file = [load_tile_file](const QString &file_path)
	{
		load_tile_file(file_path, comper::decode);
	};

	const auto load_palette_file = [this](const QString &file_path, const int starting_palette_line)
	{
		if (!file_path.isNull())
		{
			QFile file(file_path);
			if (!file.open(QFile::ReadOnly))
			{
				QMessageBox::critical(this, "Error", "Failed to load file: file could not be opened for reading.");
				return;
			}

			DataStream stream(&file);

			palette.modify(
				[&stream, starting_palette_line](Palette &palette)
				{
					palette.fromDataStream(stream, starting_palette_line);
				}
			);
		}
	};

	const auto load_sprite_mappings_file = [this](const QString &file_path)
	{
		if (!file_path.isNull())
		{
			QFile file(file_path);
			if (!file.open(QFile::ReadOnly))
			{
				QMessageBox::critical(this, "Error", "Failed to load file: file could not be opened for reading.");
				return;
			}

			sprite_mappings.modify(
				[&file](SpriteMappings &mappings)
				{
					mappings.fromFile(file);
				}
			);

			sprite_viewer.setSelectedSprite(0);
		}
	};

	const auto load_dynamic_pattern_load_cue_file = [this](const QString &file_path)
	{
		if (!file_path.isNull())
		{
			QFile file(file_path);
			if (!file.open(QFile::ReadOnly))
			{
				QMessageBox::critical(this, "Error", "Failed to load file: file could not be opened for reading.");
				return;
			}

			sprite_mappings.modify(
				[&file](SpriteMappings &mappings)
				{
					mappings.applyDPLCs(DynamicPatternLoadCues(file));
				}
			);
		}
	};

	connect(ui->actionLoad_Tiles_Uncompressed, &QAction::triggered, this,
		[this, load_uncompressed_tile_file]()
		{
			load_uncompressed_tile_file(QFileDialog::getOpenFileName(this, "Open Tile Graphics File"));
		}
	);

	connect(ui->actionLoad_Tiles_Nemesis, &QAction::triggered, this,
		[this, load_nemesis_tile_file]()
		{
			load_nemesis_tile_file(QFileDialog::getOpenFileName(this, "Open Nemesis-Compressed Tile Graphics File"));
		}
	);

	connect(ui->actionLoad_Tiles_Kosinski, &QAction::triggered, this,
		[this, load_kosinski_tile_file]()
		{
			load_kosinski_tile_file(QFileDialog::getOpenFileName(this, "Open Kosinski-Compressed Tile Graphics File"));
		}
	);

	connect(ui->actionLoad_Tiles_Moduled_Kosinski, &QAction::triggered, this,
		[this, load_moduled_kosinski_tile_file]()
		{
			load_moduled_kosinski_tile_file(QFileDialog::getOpenFileName(this, "Open Moduled Kosinski-Compressed Tile Graphics File"));
		}
	);

	connect(ui->actionLoad_Tiles_Kosinski_Plus, &QAction::triggered, this,
		[this, load_kosinski_plus_tile_file]()
		{
			load_kosinski_plus_tile_file(QFileDialog::getOpenFileName(this, "Open Kosinski+-Compressed Tile Graphics File"));
		}
	);

	connect(ui->actionLoad_Tiles_Moduled_Kosinski_Plus, &QAction::triggered, this,
		[this, load_moduled_kosinski_plus_tile_file]()
		{
			load_moduled_kosinski_plus_tile_file(QFileDialog::getOpenFileName(this, "Open Moduled Kosinski+-Compressed Tile Graphics File"));
		}
	);

	connect(ui->actionLoad_Tiles_Comper, &QAction::triggered, this,
		[this, load_comper_tile_file]()
		{
			load_comper_tile_file(QFileDialog::getOpenFileName(this, "Open Comper-Compressed Tile Graphics File"));
		}
	);

	connect(ui->actionLoad_Primary_Palette, &QAction::triggered, this,
		[this, load_palette_file]()
		{
			load_palette_file(QFileDialog::getOpenFileName(this, "Open Palette File"), 0);
		}
	);

	connect(ui->actionLoad_Secondary_Palette_Lines, &QAction::triggered, this,
		[this, load_palette_file]()
		{
			load_palette_file(QFileDialog::getOpenFileName(this, "Open Palette File"), 1);
		}
	);

	connect(ui->actionLoad_Mappings, &QAction::triggered, this,
		[this, load_sprite_mappings_file]()
		{
			load_sprite_mappings_file(QFileDialog::getOpenFileName(this, "Open Sprite Mappings File"));
		}
	);

	connect(ui->actionLoad_Sprite_Pattern_Cues, &QAction::triggered, this,
		[this, load_dynamic_pattern_load_cue_file]()
		{
			load_dynamic_pattern_load_cue_file(QFileDialog::getOpenFileName(this, "Open Dynamic Pattern Loading Cue File"));
		}
	);

	connect(ui->actionExit, &QAction::triggered, this, &QMainWindow::close);

	//////////////////////////////////
	// Menubar: File/Save Data File //
	//////////////////////////////////

	const auto save_tile_file = [this](const QString &prompt, bool (* const callback)(std::istream &in, std::ostream &out))
	{
		const QString file_path = QFileDialog::getSaveFileName(this, prompt);

		std::ofstream file(file_path.toStdString(), std::ofstream::out | std::ofstream::binary);

		if (!file.is_open())
		{
			QMessageBox::critical(this, "Error", "Failed to save file: file could not be opened for writing.");
			return;
		}

		const auto &tile_bytes = tile_manager.tile_bytes();

		// TODO: This copy should not be necessary.
		// Maybe I should modify mdcomp to use a better interface.
		std::stringstream string_stream;
		string_stream.write(reinterpret_cast<const char*>(tile_bytes.data()->data()), tile_bytes.size() * tile_bytes.data()->size());

		if (!callback(string_stream, file))
		{
			QMessageBox::critical(this, "Error", "Failed to save file: data could not be compressed.");
			return;
		}
	};

	connect(ui->actionUncompressed, &QAction::triggered, this,
		[save_tile_file]()
		{
			save_tile_file("Save Tile Graphics File",
				[](std::istream &in, std::ostream &out)
				{
					out << in.rdbuf();

					return true;
				}
			);
		}
	);

	connect(ui->actionNemesis, &QAction::triggered, this,
		[save_tile_file]()
		{
			save_tile_file("Save Nemesis-Compressed Tile Graphics File", nemesis::encode);
		}
	);

	connect(ui->actionKosinski, &QAction::triggered, this,
		[save_tile_file]()
		{
			save_tile_file("Save Kosinski-Compressed Tile Graphics File", kosinski::encode);
		}
	);

	connect(ui->actionModuled_Kosinski, &QAction::triggered, this,
		[this, save_tile_file]()
		{
			if (tile_manager.tile_bytes().size() * tile_manager.tile_bytes().data()->size() > 0xFFFF)
			{
				QMessageBox::critical(this, "Error", "Tile data is too large for moduled compression.");
				return;
			}

			save_tile_file("Save Moduled Kosinski-Compressed Tile Graphics File", [](std::istream &in, std::ostream &out){return kosinski::moduled_encode(in, out);});
		}
	);

	connect(ui->actionKosinski_2, &QAction::triggered, this,
		[save_tile_file]()
		{
			save_tile_file("Save Kosinski+-Compressed Tile Graphics File", kosplus::encode);
		}
	);

	connect(ui->actionModuled_Kosinski_2, &QAction::triggered, this,
		[this, save_tile_file]()
		{
			if (tile_manager.tile_bytes().size() * tile_manager.tile_bytes().data()->size() > 0xFFFF)
			{
				QMessageBox::critical(this, "Error", "Tile data is too large for moduled compression.");
				return;
			}

			save_tile_file("Save Moduled Kosinski+-Compressed Tile Graphics File", [](std::istream &in, std::ostream &out){return kosplus::moduled_encode(in, out);});
		}
	);

	connect(ui->actionComper, &QAction::triggered, this,
		[save_tile_file]()
		{
			save_tile_file("Save Comper-Compressed Tile Graphics File", comper::encode);
		}
	);

	const auto save_palette_file = [this](const int starting_palette_line, const int ending_palette_line)
	{
		const QString file_path = QFileDialog::getSaveFileName(this, "Save Palette File");

		if (file_path.isNull())
			return;

		QFile file(file_path);
		if (!file.open(QFile::OpenModeFlag::WriteOnly))
			QMessageBox::critical(this, "Error", "Failed to save file: file could not be opened for writing.");

		DataStream stream(&file);

		for (int i = starting_palette_line; i < ending_palette_line; ++i)
			palette->lines[i].toDataStream(stream);
	};

	connect(ui->actionSave_Primary_Palette_Line, &QAction::triggered, this,
		[save_palette_file]()
		{
			save_palette_file(0, 1);
		}
	);

	connect(ui->actionSave_Secondary_Palette_Lines, &QAction::triggered, this,
		[save_palette_file]()
		{
			save_palette_file(1, Palette::TOTAL_LINES);
		}
	);

	connect(ui->actionSave_Full_Palette, &QAction::triggered, this,
		[save_palette_file]()
		{
			save_palette_file(0, Palette::TOTAL_LINES);
		}
	);

	connect(ui->actionSave_Mappings, &QAction::triggered, this,
		[this]()
		{
			const QString file_path = QFileDialog::getSaveFileName(this, "Save Sprite Mappings File");

			if (file_path.isNull())
				return;

			QFile file(file_path);
			if (!file.open(QFile::OpenModeFlag::WriteOnly))
				QMessageBox::critical(this, "Error", "Failed to save file: file could not be opened for writing.");

			DataStream stream(&file);

			auto sprite_mappings_copy = *sprite_mappings;
			sprite_mappings_copy.removeDPLCs();
			sprite_mappings_copy.toDataStream(stream);
		}
	);

	connect(ui->actionSave_Pattern_Cues, &QAction::triggered, this,
		[this]()
		{
			// TODO: This file-opening code is repeated a few times, so see if I can move it to a function.
			const QString file_path = QFileDialog::getSaveFileName(this, "Save Dynamic Pattern Loading Cue File");

			if (file_path.isNull())
				return;

			QFile file(file_path);
			if (!file.open(QFile::OpenModeFlag::WriteOnly))
				QMessageBox::critical(this, "Error", "Failed to save file: file could not be opened for writing.");

			DataStream stream(&file);

			auto sprite_mappings_copy = *sprite_mappings;
			sprite_mappings_copy.removeDPLCs().toDataStream(stream);
		}
	);

	///////////////////////////////
	// Menubar: File/Unload Data //
	///////////////////////////////

	connect(ui->actionUnload_Tile_Graphics, &QAction::triggered, &tile_manager, &TileManager::unloadTiles);
	connect(ui->actionUnload_Palette, &QAction::triggered, this,
		[this]()
		{
			palette.modify(
				[](Palette &palette)
				{
					palette.reset();
				}
			);
		}
	);
	connect(ui->actionUnload_Mappings, &QAction::triggered, this,
		[this]()
		{
			sprite_mappings.modify(
				[](SpriteMappings &mappings)
				{
					mappings.frames.clear();
				}
			);
		}
	);
	connect(ui->actionUnload_Pattern_Cues, &QAction::triggered, this,
		[this]()
		{
			sprite_mappings.modify(
				[](SpriteMappings &mappings)
				{
					mappings.removeDPLCs();
				}
			);
		}
	);
	connect(ui->actionUnload_All, &QAction::triggered, this,
		[this]()
		{
			ui->actionUnload_Tile_Graphics->trigger();
			ui->actionUnload_Palette->trigger();
			ui->actionUnload_Mappings->trigger();
			ui->actionUnload_Pattern_Cues->trigger();
		}
	);

	///////////////////////////////////
	// Menubar: File/Export to Image //
	///////////////////////////////////

	connect(ui->actionRender_Sprite_Image, &QAction::triggered, this,
		[this]()
		{
			const QString file_path = QFileDialog::getSaveFileName(this, "Render Sprite Frame", QString(), "Image (*.png *.bmp)");

			if (file_path.isNull())
				return;

			const auto &frame = sprite_mappings->frames[sprite_viewer.selected_sprite_index()];
			const auto &frame_rect = frame.rect();

			QImage image(frame_rect.width(), frame_rect.height(), QImage::Format_RGB32);

			// Fill image with a reserved colour (this indicates
			// transparency and should never occur in a sprite).
			image.fill(QColor(0xFF, 0, 0xFF));

			// Render the sprite onto the image.
			QPainter painter(&image);
			painter.translate(-frame_rect.left(), -frame_rect.top());
			frame.draw(painter, tile_manager, TileManager::PixmapType::NO_BACKGROUND, 0, TileManager::PixmapType::NO_BACKGROUND, sprite_viewer.starting_palette_line());

			// Save the image to disk.
			if (!image.save(file_path))
				QMessageBox::critical(this, "Error", "Failed to export image.");
		}
	);

	//////////////////////////
	// Menubar: Edit/Sprite //
	//////////////////////////

	connect(ui->actionNext_Sprite, &QAction::triggered, this, [this](){sprite_viewer.setSelectedSprite(sprite_viewer.selected_sprite_index() + 1);});
	connect(ui->actionPrevious_Sprite, &QAction::triggered, this, [this](){sprite_viewer.setSelectedSprite(sprite_viewer.selected_sprite_index() - 1);});

	connect(ui->actionSwap_Sprite_with_Next, &QAction::triggered, this,
		[this]()
		{
			sprite_mappings.modify(
				[this](SpriteMappings &mappings)
				{
					auto &frames = mappings.frames;
					const int selected_sprite_index = sprite_viewer.selected_sprite_index();

					frames.move(selected_sprite_index, selected_sprite_index + 1);
					sprite_viewer.setSelectedSprite(selected_sprite_index + 1);
				}
			);
		}
	);

	connect(ui->actionSwap_Sprite_with_Previous, &QAction::triggered, this,
		[this]()
		{
			sprite_mappings.modify(
				[this](SpriteMappings &mappings)
				{
					auto &frames = mappings.frames;
					const int selected_sprite_index = sprite_viewer.selected_sprite_index();

					frames.move(selected_sprite_index, selected_sprite_index - 1);
					sprite_viewer.setSelectedSprite(selected_sprite_index - 1);
				}
			);
		}
	);

	connect(ui->actionFirst_Sprite, &QAction::triggered, this,
		[this]()
		{
			sprite_viewer.setSelectedSprite(0);
		}
	);

	connect(ui->actionLast_Sprite, &QAction::triggered, this,
		[this]()
		{
			sprite_viewer.setSelectedSprite(sprite_mappings->frames.size() - 1);
		}
	);

	connect(ui->actionInsert_New_Sprite, &QAction::triggered, this,
		[this]()
		{
			sprite_mappings.modify(
				[this](SpriteMappings &mappings)
				{
					mappings.frames.insert(sprite_viewer.selected_sprite_index() + 1, SpriteFrame());
				}
			);

			sprite_viewer.setSelectedSprite(sprite_viewer.selected_sprite_index() + 1);
		}
	);

	connect(ui->actionDuplicate_Sprite, &QAction::triggered, this,
		[this]()
		{
			sprite_mappings.modify(
				[this](SpriteMappings &mappings)
				{
					auto &frames = mappings.frames;
					const int selected_sprite_index = sprite_viewer.selected_sprite_index();

					frames.insert(selected_sprite_index + 1, frames[selected_sprite_index]);
					sprite_viewer.setSelectedSprite(selected_sprite_index + 1);
				}
			);
		}
	);

	connect(ui->actionDelete_Sprite, &QAction::triggered, this,
		[this]()
		{
			sprite_mappings.modify(
				[this](SpriteMappings &mappings)
				{
					mappings.frames.remove(sprite_viewer.selected_sprite_index());
				}
			);
		}
	);

	/////////////////////////
	// Menubar: Edit/Piece //
	/////////////////////////

	connect(ui->actionNext_Sprite_Piece, &QAction::triggered, this,
		[this]()
		{
			sprite_viewer.setSelectedPiece(sprite_viewer.selected_piece_index() + 1);
		}
	);

	connect(ui->actionPrevious_Sprite_Piece, &QAction::triggered, this,
		[this]()
		{
			sprite_viewer.setSelectedPiece(sprite_viewer.selected_piece_index() - 1);
		}
	);

	connect(ui->actionDelete_Sprite_Piece, &QAction::triggered, this,
		[this]()
		{
			sprite_viewer.setSelectedPiece(sprite_viewer.selected_piece_index() - 1);

			sprite_mappings.modify(
				[this](SpriteMappings &mappings)
				{
					mappings.frames[sprite_viewer.selected_sprite_index()].pieces.remove(sprite_viewer.selected_piece_index() + 1);
				}
			);
		}
	);

	connect(ui->actionDuplicate_Sprite_Piece, &QAction::triggered, this,
		[this]()
		{
			sprite_mappings.modify(
				[this](SpriteMappings &mappings)
				{
					auto &pieces = mappings.frames[sprite_viewer.selected_sprite_index()].pieces;
					const int selected_piece_index = sprite_viewer.selected_piece_index();

					pieces.insert(selected_piece_index + 1, pieces[selected_piece_index]);
					sprite_viewer.setSelectedPiece(selected_piece_index + 1);
				}
			);
		}
	);

	connect(ui->actionMove_Piece_into_Next_Sprite, &QAction::triggered, this,
		[this]()
		{
			sprite_mappings.modify(
				[this](SpriteMappings &mappings)
				{
					const int selected_sprite_index = sprite_viewer.selected_sprite_index();
					const int next_sprite_index = selected_sprite_index + 1;
					const int selected_piece_index = sprite_viewer.selected_piece_index();
					auto &frames = mappings.frames;
					auto &pieces = frames[selected_sprite_index].pieces;

					frames[next_sprite_index].pieces.append(pieces[selected_piece_index]);
					pieces.remove(selected_piece_index);

					sprite_viewer.setSelectedSprite(next_sprite_index);
					sprite_viewer.setSelectedPiece(frames[next_sprite_index].pieces.size() - 1);
				}
			);
		}
	);

	connect(ui->actionMove_Piece_into_Previous_Sprite, &QAction::triggered, this,
		[this]()
		{
			sprite_mappings.modify(
				[this](SpriteMappings &mappings)
				{
					const int selected_sprite_index = sprite_viewer.selected_sprite_index();
					const int previous_sprite_index = selected_sprite_index - 1;
					const int selected_piece_index = sprite_viewer.selected_piece_index();
					auto &frames = mappings.frames;
					auto &pieces = frames[selected_sprite_index].pieces;

					frames[previous_sprite_index].pieces.append(pieces[selected_piece_index]);
					pieces.remove(selected_piece_index);

					sprite_viewer.setSelectedSprite(previous_sprite_index);
					sprite_viewer.setSelectedPiece(frames[previous_sprite_index].pieces.size() - 1);
				}
			);
		}
	);

	connect(ui->actionMove_Piece_into_New_Sprite, &QAction::triggered, this,
		[this]()
		{
			sprite_mappings.modify(
				[this](SpriteMappings &mappings)
				{
					const int selected_sprite_index = sprite_viewer.selected_sprite_index();
					const int next_sprite_index = selected_sprite_index + 1;
					const int selected_piece_index = sprite_viewer.selected_piece_index();
					auto &frames = mappings.frames;
					auto &pieces = frames[selected_sprite_index].pieces;

					frames.insert(next_sprite_index, SpriteFrame());
					frames[next_sprite_index].pieces.append(pieces[selected_piece_index]);
					pieces.remove(selected_piece_index);

					sprite_viewer.setSelectedSprite(next_sprite_index);
					sprite_viewer.setSelectedPiece(frames[next_sprite_index].pieces.size() - 1);
				}
			);
		}
	);

	connect(ui->actionMove_Piece_toward_Back, &QAction::triggered, this,
		[this]()
		{
			sprite_mappings.modify(
				[this](SpriteMappings &mappings)
				{
					auto &pieces = mappings.frames[sprite_viewer.selected_sprite_index()].pieces;
					const int selected_piece_index = sprite_viewer.selected_piece_index();
					const int next_piece_index = selected_piece_index + 1;

					pieces.move(selected_piece_index, next_piece_index);

					sprite_viewer.setSelectedPiece(next_piece_index);
				}
			);
		}
	);

	connect(ui->actionMove_Piece_toward_Front, &QAction::triggered, this,
		[this]()
		{
			sprite_mappings.modify(
				[this](SpriteMappings &mappings)
				{
					auto &pieces = mappings.frames[sprite_viewer.selected_sprite_index()].pieces;
					const int selected_piece_index = sprite_viewer.selected_piece_index();
					const int previous_piece_index = selected_piece_index - 1;

					pieces.move(selected_piece_index, previous_piece_index);

					sprite_viewer.setSelectedPiece(previous_piece_index);
				}
			);
		}
	);

	connect(ui->actionMove_Piece_Behind_Others, &QAction::triggered, this,
		[this]()
		{
			sprite_mappings.modify(
				[this](SpriteMappings &mappings)
				{
					auto &pieces = mappings.frames[sprite_viewer.selected_sprite_index()].pieces;
					const int selected_piece_index = sprite_viewer.selected_piece_index();
					const int last_piece_index = pieces.size() - 1;

					pieces.move(selected_piece_index, last_piece_index);

					sprite_viewer.setSelectedPiece(last_piece_index);
				}
			);
		}
	);

	connect(ui->actionMove_Piece_in_Front_of_Others, &QAction::triggered, this,
		[this]()
		{
			sprite_mappings.modify(
				[this](SpriteMappings &mappings)
				{
					auto &pieces = mappings.frames[sprite_viewer.selected_sprite_index()].pieces;
					const int selected_piece_index = sprite_viewer.selected_piece_index();
					const int first_piece_index = 0;

					pieces.move(selected_piece_index, first_piece_index);

					sprite_viewer.setSelectedPiece(first_piece_index);
				}
			);
		}
	);

	//////////////////////////////////
	// Menubar: Edit/Transformation //
	//////////////////////////////////

	const auto transform_frame_or_piece = [this](const std::function<void(SpritePiece &piece)> &callback)
	{
		sprite_mappings.modify(
			[this, &callback](SpriteMappings &mappings)
			{
				auto &frame = mappings.frames[sprite_viewer.selected_sprite_index()];
				const int piece_index = sprite_viewer.selected_piece_index();

				if (piece_index != -1)
				{
					auto &piece = frame.pieces[piece_index];
					callback(piece);
				}
				else
				{
					for (auto &piece : frame.pieces)
						callback(piece);
				}
			}
		);
	};

	connect(ui->actionFlip_Horizontally, &QAction::triggered, this,
		[this, transform_frame_or_piece]()
		{
			const auto &frame = sprite_mappings->frames[sprite_viewer.selected_sprite_index()];
			const QRect &rect = sprite_viewer.selected_piece_index() == -1 ? frame.rect() : frame.pieces[sprite_viewer.selected_piece_index()].rect();

			transform_frame_or_piece(
				[&rect](SpritePiece &piece)
				{
					piece.x = rect.left() + (rect.width() - (piece.x - rect.left()) - piece.width * TileManager::TILE_WIDTH);
					piece.x_flip = !piece.x_flip;
				}
			);
		}
	);

	connect(ui->actionFlip_Vertically, &QAction::triggered, this,
		[this, transform_frame_or_piece]()
		{
			const auto &frame = sprite_mappings->frames[sprite_viewer.selected_sprite_index()];
			const QRect &rect = sprite_viewer.selected_piece_index() == -1 ? frame.rect() : frame.pieces[sprite_viewer.selected_piece_index()].rect();

			transform_frame_or_piece(
				[&rect](SpritePiece &piece)
				{
					piece.y = rect.top() + (rect.height() - (piece.y - rect.top()) - piece.height * TileManager::TILE_HEIGHT);
					piece.y_flip = !piece.y_flip;
				}
			);
		}
	);

	connect(ui->actionCycle_Palette, &QAction::triggered, this,
		[transform_frame_or_piece]()
		{
			transform_frame_or_piece(
				[](SpritePiece &piece)
				{
					piece.palette_line = (piece.palette_line + 1) % 4;
				}
			);
		}
	);

	connect(ui->actionToggle_Foreground_Flag, &QAction::triggered, this,
		[transform_frame_or_piece]()
		{
			transform_frame_or_piece(
				[](SpritePiece &piece)
				{
					piece.priority = !piece.priority;
				}
			);
		}
	);

	const auto move_frame_or_piece = [transform_frame_or_piece](const int x, const int y)
	{
		transform_frame_or_piece(
			[x, y](SpritePiece &piece)
			{
				piece.x += x; piece.y += y;
			}
		);
	};

	connect(ui->actionMove_Left_8_Pixels, &QAction::triggered, this, [move_frame_or_piece](){move_frame_or_piece(-8, 0);});
	connect(ui->actionMove_Right_8_Pixels, &QAction::triggered, this, [move_frame_or_piece](){move_frame_or_piece(8, 0);});
	connect(ui->actionMove_Up_8_Pixels, &QAction::triggered, this, [move_frame_or_piece](){move_frame_or_piece(0, -8);});
	connect(ui->actionMove_Down_8_Pixels, &QAction::triggered, this, [move_frame_or_piece](){move_frame_or_piece(0, 8);});

	connect(ui->actionMove_Left_1_Pixel, &QAction::triggered, this, [move_frame_or_piece](){move_frame_or_piece(-1, 0);});
	connect(ui->actionMove_Right_1_Pixel, &QAction::triggered, this, [move_frame_or_piece](){move_frame_or_piece(1, 0);});
	connect(ui->actionMove_Up_1_Pixel, &QAction::triggered, this, [move_frame_or_piece](){move_frame_or_piece(0, -1);});
	connect(ui->actionMove_Down_1_Pixel, &QAction::triggered, this, [move_frame_or_piece](){move_frame_or_piece(0, 1);});

	////////////////
	// Edit/Tiles //
	////////////////

	const auto set_tile_selection_and_update_piece_picker = [this](const std::function<void(QVector<bool> &selected)> &callback)
	{
		tile_viewer.setSelection(true, callback);

		const int first_selected_tile = tile_viewer.selection().indexOf(true);

		if (first_selected_tile != -1)
			sprite_piece_picker.setSelectedTile(first_selected_tile);
	};

	connect(ui->actionSelect_All_Unmapped_Tiles, &QAction::triggered, this,
		[this, set_tile_selection_and_update_piece_picker]()
		{
			set_tile_selection_and_update_piece_picker(
				[this](QVector<bool> &selected)
				{
					for (auto &tile_selected : selected)
						tile_selected = true;

					for (const auto &frame : qAsConst(sprite_mappings->frames))
						for (const auto &piece : frame.pieces)
							for (int i = 0; i < piece.width * piece.height; ++i)
								selected[piece.tile_index + i] = false;
				}
			);
		}
	);

	connect(ui->actionDelete_Selected_Tiles, &QAction::triggered, this,
		[this]()
		{
			tile_viewer.setSelection(false,
				[this](QVector<bool> &selected)
				{
					sprite_mappings.modify(
						[this, &selected](SpriteMappings &mappings)
						{
							int selected_tile;
							while ((selected_tile = selected.lastIndexOf(true)) != -1)
							{
								// Un-select the tile, to prevent an infinite loop.
								selected[selected_tile] = false;

								// Remove the tile.
								tile_manager.deleteTile(selected_tile);

								// Correct sprite piece tile indices to account for the removed tile.
								for (auto &frame : mappings.frames)
									for (auto &piece : frame.pieces)
										if (piece.tile_index > selected_tile)
											--piece.tile_index;
							}
						}
					);
				}
			);
		}
	);

	connect(ui->actionFind_Sprite_that_Uses_Focused_Tile, &QAction::triggered, this,
		[this]()
		{
			const int selected_tile_index = tile_viewer.selection().indexOf(true);

			const auto &frames = sprite_mappings->frames;
			for (int frame_index = 0; frame_index < frames.size(); ++frame_index)
			{
				const auto &frame = frames[frame_index];

				// TODO: Improve this.
				// This algorithm sucks, but it's exactly what SonMapEd does.
				// The problem with this is that it only toggles between the first
				// two sprites to use the selected tile, when it would be better if
				// it cycled between every sprite that uses the tile instead.
				if (frame_index != sprite_viewer.selected_sprite_index())
				{
					for (int piece_index = 0; piece_index < frame.pieces.size(); ++piece_index)
					{
						const auto &piece = frame.pieces[piece_index];

						if (selected_tile_index >= piece.tile_index && selected_tile_index < piece.tile_index + piece.width * piece.height)
						{
							// TODO: Don't do this.
							// This is a gross hack: back up the selection so that we can
							// restore it after changing the selected sprite.
							const auto selection = tile_viewer.selection();
							const int piece_picker_tile =  sprite_piece_picker.selected_tile();

							sprite_viewer.setSelectedSprite(frame_index);
							sprite_viewer.setSelectedPiece(piece_index);

							// Restore the selection.
							tile_viewer.setSelection(false,
								[&selection](QVector<bool> &selected)
								{
									selected = selection;
								}
							);

							sprite_piece_picker.setSelectedTile(piece_picker_tile);

							return;
						}
					}
				}
			}
		}
	);

	connect(ui->actionDuplicate_Selected_Tiles, &QAction::triggered, this,
		[this]()
		{
			tile_viewer.setSelection(false,
				[this](QVector<bool> &selected)
				{
					sprite_mappings.modify(
						[this, &selected](SpriteMappings &mappings)
						{
							int selected_tile = -1;
							while (selected_tile != 0 && (selected_tile = selected.lastIndexOf(true, selected_tile - 1)) != -1)
							{
								// Duplicate the tile.
								const int insertion_index = selected.indexOf(false, selected_tile);
								tile_manager.duplicateTile(selected_tile, insertion_index == -1 ? selected.size() : insertion_index);

								// Correct sprite piece tile indices to account for the inserted tile.
								for (auto &frame : mappings.frames)
									for (auto &piece : frame.pieces)
										if (piece.tile_index >= insertion_index)
											++piece.tile_index;
							}
						}
					);
				}
			);

			tile_viewer.clearSelection();
		}
	);

	connect(ui->actionClear_Pixels_of_Selected_Tiles, &QAction::triggered, this,
		[this]()
		{
			const auto &selection = tile_viewer.selection();

			for (int tile_index = 0; tile_index < selection.size(); ++tile_index)
				if (selection[tile_index])
					tile_manager.clearTile(tile_index);
		}
	);

	connect(ui->actionInvert_Selection, &QAction::triggered, this,
		[this]()
		{
			tile_viewer.setSelection(false,
				[](QVector<bool> &selected)
				{
					for (auto &tile_selected : selected)
						tile_selected = !tile_selected;
				}
			);
		}
	);

	connect(ui->actionSelect_Tiles_of_Active_Pieces, &QAction::triggered, this,
		[this, set_tile_selection_and_update_piece_picker]()
		{
			tile_viewer.clearSelection();
			set_tile_selection_and_update_piece_picker(
				[this](QVector<bool> &selected)
				{
					for (const auto &piece : qAsConst(sprite_mappings->frames[sprite_viewer.selected_sprite_index()].pieces))
						for (int i = 0; i < piece.width * piece.height; ++i)
							selected[piece.tile_index + i] = true;
				}
			);
		}
	);

	/////////////////
	// View/Scroll //
	/////////////////

	connect(ui->actionScroll_Back_1_Row_Column, &QAction::triggered, this,
		[this]()
		{
			int tiles_per_row, total_rows;
			tile_viewer.getGridDimensions(tiles_per_row, total_rows);
			Q_UNUSED(total_rows);
			tile_viewer.setScroll(tile_viewer.scroll() - tiles_per_row * 1);
		}
	);

	connect(ui->actionScroll_Forward_1_Row_Column, &QAction::triggered, this,
		[this]()
		{
			int tiles_per_row, total_rows;
			tile_viewer.getGridDimensions(tiles_per_row, total_rows);

			Q_UNUSED(total_rows);
			if (tile_viewer.scroll() < tile_manager.total_tiles() - tiles_per_row * 1)
				tile_viewer.setScroll(tile_viewer.scroll() + tiles_per_row * 1);
		}
	);

	connect(ui->actionScroll_Back_1_Tile, &QAction::triggered, this,
		[this]()
		{
			tile_viewer.setScroll(tile_viewer.scroll() - 1);
		}
	);

	connect(ui->actionScroll_Forward_1_Tile, &QAction::triggered, this,
		[this]()
		{
			tile_viewer.setScroll(tile_viewer.scroll() + 1);
		}
	);

	connect(ui->actionScroll_Back_8_Rows_Columns, &QAction::triggered, this,
		[this]()
		{
			int tiles_per_row, total_rows;
			tile_viewer.getGridDimensions(tiles_per_row, total_rows);
			Q_UNUSED(total_rows);
			tile_viewer.setScroll(tile_viewer.scroll() - tiles_per_row * 8);
		}
	);

	connect(ui->actionScroll_Forward_8_Rows_Columns, &QAction::triggered, this,
		[this]()
		{
			int tiles_per_row, total_rows;
			tile_viewer.getGridDimensions(tiles_per_row, total_rows);

			Q_UNUSED(total_rows);
			if (tile_viewer.scroll() < tile_manager.total_tiles() - tiles_per_row * 8)
				tile_viewer.setScroll(tile_viewer.scroll() + tiles_per_row * 8);
		}
	);

	connect(ui->actionScroll_Back_8_Tiles, &QAction::triggered, this,
		[this]()
		{
			tile_viewer.setScroll(tile_viewer.scroll() - 8);
		}
	);

	connect(ui->actionScroll_Forward_8_Tiles, &QAction::triggered, this,
		[this]()
		{
			tile_viewer.setScroll(tile_viewer.scroll() + 8);
		}
	);

	/////////////////////////////
	// Settings/Tile Rendering //
	/////////////////////////////

	const auto set_starting_palette_line = [this](const int line)
	{
		sprite_piece_picker.setPaletteLine(line);
		sprite_viewer.setStartingPaletteLine(line);
		tile_viewer.setPaletteLine(line);

		ui->actionRender_Starting_with_Palette_Line_1->setChecked(line == 0);
		ui->actionRender_Starting_with_Palette_Line_2->setChecked(line == 1);
		ui->actionRender_Starting_with_Palette_Line_3->setChecked(line == 2);
		ui->actionRender_Starting_with_Palette_Line_4->setChecked(line == 3);
	};

	connect(ui->actionRender_Starting_with_Palette_Line_1, &QAction::triggered, this, [set_starting_palette_line](){set_starting_palette_line(0);});
	connect(ui->actionRender_Starting_with_Palette_Line_2, &QAction::triggered, this, [set_starting_palette_line](){set_starting_palette_line(1);});
	connect(ui->actionRender_Starting_with_Palette_Line_3, &QAction::triggered, this, [set_starting_palette_line](){set_starting_palette_line(2);});
	connect(ui->actionRender_Starting_with_Palette_Line_4, &QAction::triggered, this, [set_starting_palette_line](){set_starting_palette_line(3);});

	set_starting_palette_line(0);

	////////////////////////
	// Menubar Activation //
	////////////////////////

	const auto update_menubar = [this]()
	{
		const int selected_sprite_index = sprite_viewer.selected_sprite_index();
		const bool no_sprites = sprite_mappings->frames.size() == 0;
		const bool is_first_sprite = no_sprites || selected_sprite_index == 0;
		const bool is_last_sprite = no_sprites || selected_sprite_index == sprite_mappings->frames.size() - 1;
		const bool no_sprite_selected = no_sprites;

		ui->actionRender_Sprite_Image->setDisabled(no_sprite_selected);

		ui->actionNext_Sprite->setDisabled(is_last_sprite);
		ui->actionPrevious_Sprite->setDisabled(is_first_sprite);
		ui->actionSwap_Sprite_with_Next->setDisabled(is_last_sprite);
		ui->actionSwap_Sprite_with_Previous->setDisabled(is_first_sprite);
		ui->actionPrevious_Sprite->setDisabled(is_first_sprite);
		ui->actionFirst_Sprite->setDisabled(is_first_sprite);
		ui->actionLast_Sprite->setDisabled(is_last_sprite);
		ui->actionDuplicate_Sprite->setDisabled(no_sprite_selected);
		ui->actionDelete_Sprite->setDisabled(no_sprite_selected);

		const int selected_piece_index = sprite_viewer.selected_piece_index();
		const bool no_pieces = no_sprites || sprite_mappings->frames[selected_sprite_index].pieces.size() == 0;
		const bool is_first_piece = no_pieces || selected_piece_index == 0;
		const bool is_last_piece = no_pieces || selected_piece_index == sprite_mappings->frames[selected_sprite_index].pieces.size() - 1;
		const bool no_piece_selected = no_pieces || selected_piece_index == -1;

		ui->actionNext_Sprite_Piece->setDisabled(no_pieces);
		ui->actionPrevious_Sprite_Piece->setDisabled(no_pieces);
		ui->actionDuplicate_Sprite_Piece->setDisabled(no_piece_selected);
		ui->actionDelete_Sprite_Piece->setDisabled(no_piece_selected);
		ui->actionMove_Piece_into_Next_Sprite->setDisabled(no_piece_selected || is_last_sprite);
		ui->actionMove_Piece_into_Previous_Sprite->setDisabled(no_piece_selected || is_first_sprite);
		ui->actionMove_Piece_into_New_Sprite->setDisabled(no_piece_selected || is_last_sprite);
		ui->actionMove_Piece_toward_Back->setDisabled(is_last_piece);
		ui->actionMove_Piece_toward_Front->setDisabled(is_first_piece);
		ui->actionMove_Piece_Behind_Others->setDisabled(is_last_piece);
		ui->actionMove_Piece_in_Front_of_Others->setDisabled(is_first_piece);

		const bool nothing_selected = no_sprite_selected;

		ui->actionFlip_Horizontally->setDisabled(nothing_selected);
		ui->actionFlip_Vertically->setDisabled(nothing_selected);
		ui->actionCycle_Palette->setDisabled(nothing_selected);
		ui->actionToggle_Foreground_Flag->setDisabled(nothing_selected);
		ui->actionMove_Left_8_Pixels->setDisabled(nothing_selected);
		ui->actionMove_Right_8_Pixels->setDisabled(nothing_selected);
		ui->actionMove_Up_8_Pixels->setDisabled(nothing_selected);
		ui->actionMove_Down_8_Pixels->setDisabled(nothing_selected);
		ui->actionMove_Left_1_Pixel->setDisabled(nothing_selected);
		ui->actionMove_Right_1_Pixel->setDisabled(nothing_selected);
		ui->actionMove_Up_1_Pixel->setDisabled(nothing_selected);
		ui->actionMove_Down_1_Pixel->setDisabled(nothing_selected);
		ui->actionSelect_Tiles_of_Active_Pieces->setDisabled(nothing_selected);
	};

	connect(&sprite_viewer, &SpriteViewer::selectedSpriteChanged, this, update_menubar);
	connect(&sprite_mappings, &SignalWrapper<SpriteMappings>::modified, this, update_menubar);

	// Manually update the menubar upon startup so that the various options are properly enabled.
	update_menubar();

	//////////////////
	// Window Title //
	//////////////////

	const auto update_title = [this]()
	{
		const auto do_string_thingy = [](const QString &string, const int index, const int total)
		{
			return " | " + string + (index == -1 ? "s: " : (" " + QString::number(index, 0x10).toUpper() + "/")) + QString::number(total, 0x10).toUpper();
		};

		const QString frame_string = do_string_thingy("Frame", sprite_viewer.selected_sprite_index(), sprite_mappings->frames.size());
		const QString piece_string = sprite_viewer.selected_sprite_index() == -1 ? "" : do_string_thingy("Piece", sprite_viewer.selected_piece_index(), sprite_mappings->frames[sprite_viewer.selected_sprite_index()].pieces.size());
		const QString tile_string = do_string_thingy("Tile", tile_viewer.selection().indexOf(true), tile_manager.total_tiles());

		setWindowTitle("ClownMapEd" + frame_string + piece_string + tile_string);
	};

	connect(&sprite_viewer, &SpriteViewer::selectedSpriteChanged, this, update_title);
	connect(&sprite_mappings, &SignalWrapper<SpriteMappings>::modified, this, update_title);
	connect(&tile_viewer, &TileViewer::tileSelected, this, update_title);

	update_title();
}

MainWindow::~MainWindow()
{
	delete ui;
}
