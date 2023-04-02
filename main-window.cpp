#include "main-window.h"
#include "./ui_main-window.h"

#include <climits>
#include <fstream>
#include <functional>

#include <QDataStream>
#include <QFile>
#include <QFileDialog>
#include <QKeyEvent>

#include "comper.h"
#include "kosinski.h"
#include "kosplus.h"
#include "nemesis.h"

#include "dynamic-pattern-load-cues.h"

MainWindow::MainWindow(QWidget* const parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
    , tile_manager(nullptr, 0, palette)
	, sprite_viewer(tile_manager, sprite_mappings_manager)
    , palette_editor(palette)
	, sprite_piece_picker(tile_manager)
    , tile_viewer(tile_manager)
{
	ui->setupUi(this);

	horizontal_layout.addWidget(&sprite_piece_picker);
	horizontal_layout.addWidget(&palette_editor);
	horizontal_layout.addWidget(&sprite_viewer);

	vertical_layout.addLayout(&horizontal_layout);
//	vbox->addsp
	vertical_layout.addWidget(&tile_viewer);

	centralWidget()->setLayout(&vertical_layout);

	sprite_viewer.setBackgroundColour(palette.colour(0, 0));
	tile_viewer.setBackgroundColour(palette.colour(0, 0));

	horizontal_layout.setMargin(vertical_layout.margin());
	vertical_layout.setMargin(0);

	///////////////////
	// Misc. Signals //
	///////////////////

	connect(&palette, &Palette::changed, this,
		[this]()
		{
			const QColor &background_colour = palette.colour(0, 0);

			sprite_viewer.setBackgroundColour(background_colour);
			tile_viewer.setBackgroundColour(background_colour);
			sprite_piece_picker.setBackgroundColour(background_colour);
		}
	);

	connect(&tile_viewer, &TileViewer::tileSelected, &sprite_piece_picker, &SpritePiecePicker::setSelectedTile);

	connect(&sprite_viewer, &SpriteViewer::selectedSpriteChanged, this,
		[this]()
		{
			int earliest_tile_index = INT_MAX;

			tile_viewer.setSelection(
				[this, &earliest_tile_index](QVector<bool> &selection)
				{
					const int frame_index = sprite_viewer.selected_sprite_index();

					if (frame_index != -1)
					{
						const auto &frames = sprite_mappings_manager.sprite_mappings().frames;
						const int piece_index = sprite_viewer.selected_piece_index();

						if (piece_index == -1)
						{
							for (auto &piece : frames[frame_index].pieces)
							{
								for (int i = 0; i < piece.width * piece.height; ++i)
									selection[piece.tile_index + i] = true;

								if (earliest_tile_index > piece.tile_index)
									earliest_tile_index = piece.tile_index;
							}
						}
						else
						{
							const auto &piece = frames[frame_index].pieces[piece_index];

							for (int i = 0; i < piece.width * piece.height; ++i)
								selection[piece.tile_index + i] = true;

							earliest_tile_index = piece.tile_index;
						}
					}
				}
			);

			if (earliest_tile_index != INT_MAX)
				sprite_piece_picker.setSelectedTile(earliest_tile_index);
		}
	);

	connect(&sprite_piece_picker, &SpritePiecePicker::pieceSelected, this,
		[this](const int width, const int height)
		{
			auto mappings = this->sprite_mappings_manager.lock();

			if (sprite_viewer.selected_sprite_index() == -1)
			{
				mappings->frames.append(SpriteFrame());
				sprite_viewer.setSelectedSprite(0);
			}

			auto &pieces = mappings->frames[sprite_viewer.selected_sprite_index()].pieces;
			pieces.append(SpritePiece{0, 0, width, height, false, 0, false, false, sprite_piece_picker.selected_tile()});
			sprite_viewer.setSelectedPiece(pieces.size() - 1);

			sprite_piece_picker.setSelectedTile(sprite_piece_picker.selected_tile() + width * height);
		}
	);

	///////////////////
	// Menubar: File //
	///////////////////

	auto load_tile_file = [this](bool (* const decompression_function)(std::istream &src, std::iostream &dst))
	{
		const QString file_path = QFileDialog::getOpenFileName(this, "Open Tile Graphics File");

		if (!file_path.isNull())
		{
			std::ifstream file_stream(file_path.toStdString(), std::ifstream::in | std::ifstream::binary);

			if (!file_stream.is_open())
				return;

			std::stringstream string_stream(std::stringstream::in | std::stringstream::out | std::stringstream::binary);

			if (!decompression_function(file_stream, string_stream))
				return;

			if (!tile_manager.setTiles(reinterpret_cast<const uchar*>(string_stream.str().c_str()), static_cast<int>(string_stream.str().length())))
			{
				// TODO: Should probably show an error message or something.
			}
		}
	};

	connect(ui->actionLoad_Tiles_Uncompressed, &QAction::triggered, this,
		[load_tile_file]()
		{
			load_tile_file([](std::istream &src, std::iostream &dst)
				{
					dst << src.rdbuf();
					return true;
				}
			);
		}
	);

	connect(ui->actionLoad_Tiles_Nemesis, &QAction::triggered, this,
		[load_tile_file]()
		{
			load_tile_file([](std::istream &src, std::iostream &dst){return nemesis::decode(src, dst);});
		}
	);

	connect(ui->actionLoad_Tiles_Kosinski, &QAction::triggered, this,
		[load_tile_file]()
		{
			load_tile_file(kosinski::decode);
		}
	);

	connect(ui->actionLoad_Tiles_Moduled_Kosinski, &QAction::triggered, this,
		[load_tile_file]()
		{
			load_tile_file([](std::istream &src, std::iostream &dst){return kosinski::moduled_decode(src, dst);});
		}
	);

	connect(ui->actionLoad_Tiles_Kosinski_Plus, &QAction::triggered, this,
		[load_tile_file]()
		{
			load_tile_file(kosplus::decode);
		}
	);

	connect(ui->actionLoad_Tiles_Moduled_Kosinski_Plus, &QAction::triggered, this,
		[load_tile_file]()
		{
			load_tile_file([](std::istream &src, std::iostream &dst){return kosplus::moduled_decode(src, dst);});
		}
	);

	connect(ui->actionLoad_Tiles_Comper, &QAction::triggered, this,
		[load_tile_file]()
		{
			load_tile_file(comper::decode);
		}
	);

	connect(ui->actionLoad_Primary_Palette, &QAction::triggered, this,
		[this]()
		{
			const QString file_path = QFileDialog::getOpenFileName(this, "Open Palette File");

			if (!file_path.isNull())
			{
				palette.loadFromFile(file_path);
				tile_manager.setPalette(palette);
				palette_editor.update();
			}
		}
	);

	connect(ui->actionLoad_Mappings, &QAction::triggered, this,
		[this]()
		{
			const QString file_path = QFileDialog::getOpenFileName(this, "Open Sprite Mappings File");

			if (!file_path.isNull())
			{
				QFile file(file_path);
				if (!file.open(QFile::ReadOnly))
					return;

				sprite_mappings_manager.lock()(SpriteMappings::fromFile(file));
				sprite_viewer.setSelectedSprite(0);
			}
		}
	);

	connect(ui->actionLoad_Sprite_Pattern_Cues, &QAction::triggered, this,
		[this]()
		{
			const QString file_path = QFileDialog::getOpenFileName(this, "Open Dynamic Pattern Loading Cue File");

			if (!file_path.isNull())
			{
				QFile file(file_path);
				if (!file.open(QFile::ReadOnly))
					return;

				const DynamicPatternLoadCues dplcs = DynamicPatternLoadCues::fromFile(file);

				auto mappings = sprite_mappings_manager.lock();

				if (mappings->frames.size() == dplcs.frames.size())
				{
					for (int frame_index = 0; frame_index < mappings->frames.size(); ++frame_index)
					{
						auto &dplc_frame = dplcs.frames[frame_index];

						for (auto &piece : mappings->frames[frame_index].pieces)
						{
							const int base_mapped_tile = dplc_frame.getMappedTile(piece.tile_index);

							for (int i = 0; i < piece.width * piece.height; ++i)
							{
								const int mapped_tile = dplc_frame.getMappedTile(piece.tile_index + i);

								if (mapped_tile == -1 || mapped_tile != base_mapped_tile + i)
									return;
							}

							piece.tile_index = base_mapped_tile;
						}
					}
				}
			}
		}
	);

	connect(ui->actionExit, &QAction::triggered, this, &QMainWindow::close);

	//////////////////////////
	// Menubar: Edit/Sprite //
	//////////////////////////

	connect(ui->actionNext_Sprite, &QAction::triggered, this, [this](){sprite_viewer.setSelectedSprite(sprite_viewer.selected_sprite_index() + 1);});
	connect(ui->actionPrevious_Sprite, &QAction::triggered, this, [this](){sprite_viewer.setSelectedSprite(sprite_viewer.selected_sprite_index() - 1);});

	connect(ui->actionSwap_Sprite_with_Next, &QAction::triggered, this,
		[this]()
		{
			auto mappings = sprite_mappings_manager.lock();
			auto &frames = mappings->frames;
			const int selected_sprite_index = sprite_viewer.selected_sprite_index();

			frames.move(selected_sprite_index, selected_sprite_index + 1);
			sprite_viewer.setSelectedSprite(selected_sprite_index + 1);
		}
	);

	connect(ui->actionSwap_Sprite_with_Previous, &QAction::triggered, this,
		[this]()
		{
			auto mappings = sprite_mappings_manager.lock();
			auto &frames = mappings->frames;
			const int selected_sprite_index = sprite_viewer.selected_sprite_index();

			frames.move(selected_sprite_index, selected_sprite_index - 1);
			sprite_viewer.setSelectedSprite(selected_sprite_index - 1);
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
			sprite_viewer.setSelectedSprite(sprite_mappings_manager.sprite_mappings().frames.size() - 1);
		}
	);

	connect(ui->actionInsert_New_Sprite, &QAction::triggered, this,
		[this]()
		{
			sprite_mappings_manager.lock()->frames.insert(sprite_viewer.selected_sprite_index() + 1, SpriteFrame());
			sprite_viewer.setSelectedSprite(sprite_viewer.selected_sprite_index() + 1);
		}
	);

	connect(ui->actionDuplicate_Sprite, &QAction::triggered, this,
		[this]()
		{
			auto mappings = sprite_mappings_manager.lock();
			auto &frames = mappings->frames;
			const int selected_sprite_index = sprite_viewer.selected_sprite_index();

			frames.insert(selected_sprite_index + 1, frames[selected_sprite_index]);
			sprite_viewer.setSelectedSprite(selected_sprite_index + 1);
		}
	);

	connect(ui->actionDelete_Sprite, &QAction::triggered, this,
		[this]()
		{
			sprite_mappings_manager.lock()->frames.remove(sprite_viewer.selected_sprite_index());
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
			sprite_mappings_manager.lock()->frames[sprite_viewer.selected_sprite_index()].pieces.remove(sprite_viewer.selected_piece_index());
			sprite_viewer.setSelectedPiece(sprite_viewer.selected_piece_index() - 1);
		}
	);

	connect(ui->actionDuplicate_Sprite_Piece, &QAction::triggered, this,
		[this]()
		{
			auto mappings = sprite_mappings_manager.lock();
			auto &pieces = mappings->frames[sprite_viewer.selected_sprite_index()].pieces;
			const int selected_piece_index = sprite_viewer.selected_piece_index();

			pieces.insert(selected_piece_index + 1, pieces[selected_piece_index]);
			sprite_viewer.setSelectedPiece(selected_piece_index + 1);
		}
	);

	connect(ui->actionMove_Piece_into_Next_Sprite, &QAction::triggered, this,
		[this]()
		{
			const int selected_sprite_index = sprite_viewer.selected_sprite_index();
			const int next_sprite_index = selected_sprite_index + 1;
			const int selected_piece_index = sprite_viewer.selected_piece_index();
			auto mappings = sprite_mappings_manager.lock();
			auto &frames = mappings->frames;
			auto &pieces = frames[selected_sprite_index].pieces;

			frames[next_sprite_index].pieces.append(pieces[selected_piece_index]);
			pieces.remove(selected_piece_index);

			sprite_viewer.setSelectedSprite(next_sprite_index);
			sprite_viewer.setSelectedPiece(frames[next_sprite_index].pieces.size() - 1);
		}
	);

	connect(ui->actionMove_Piece_into_Previous_Sprite, &QAction::triggered, this,
		[this]()
		{
			const int selected_sprite_index = sprite_viewer.selected_sprite_index();
			const int previous_sprite_index = selected_sprite_index - 1;
			const int selected_piece_index = sprite_viewer.selected_piece_index();
			auto mappings = sprite_mappings_manager.lock();
			auto &frames = mappings->frames;
			auto &pieces = frames[selected_sprite_index].pieces;

			frames[previous_sprite_index].pieces.append(pieces[selected_piece_index]);
			pieces.remove(selected_piece_index);

			sprite_viewer.setSelectedSprite(previous_sprite_index);
			sprite_viewer.setSelectedPiece(frames[previous_sprite_index].pieces.size() - 1);
		}
	);

	connect(ui->actionMove_Piece_into_New_Sprite, &QAction::triggered, this,
		[this]()
		{
			const int selected_sprite_index = sprite_viewer.selected_sprite_index();
			const int next_sprite_index = selected_sprite_index + 1;
			const int selected_piece_index = sprite_viewer.selected_piece_index();
			auto mappings = sprite_mappings_manager.lock();
			auto &frames = mappings->frames;
			auto &pieces = frames[selected_sprite_index].pieces;

			frames.insert(next_sprite_index, SpriteFrame());
			frames[next_sprite_index].pieces.append(pieces[selected_piece_index]);
			pieces.remove(selected_piece_index);

			sprite_viewer.setSelectedSprite(next_sprite_index);
			sprite_viewer.setSelectedPiece(frames[next_sprite_index].pieces.size() - 1);
		}
	);

	connect(ui->actionMove_Piece_toward_Back, &QAction::triggered, this,
		[this]()
		{
			auto mappings = sprite_mappings_manager.lock();
			auto &pieces = mappings->frames[sprite_viewer.selected_sprite_index()].pieces;
			const int selected_piece_index = sprite_viewer.selected_piece_index();
			const int next_piece_index = selected_piece_index + 1;

			pieces.move(selected_piece_index, next_piece_index);

			sprite_viewer.setSelectedPiece(next_piece_index);
		}
	);

	connect(ui->actionMove_Piece_toward_Front, &QAction::triggered, this,
		[this]()
		{
			auto mappings = sprite_mappings_manager.lock();
			auto &pieces = mappings->frames[sprite_viewer.selected_sprite_index()].pieces;
			const int selected_piece_index = sprite_viewer.selected_piece_index();
			const int previous_piece_index = selected_piece_index - 1;

			pieces.move(selected_piece_index, previous_piece_index);

			sprite_viewer.setSelectedPiece(previous_piece_index);
		}
	);

	connect(ui->actionMove_Piece_Behind_Others, &QAction::triggered, this,
		[this]()
		{
			auto mappings = sprite_mappings_manager.lock();
			auto &pieces = mappings->frames[sprite_viewer.selected_sprite_index()].pieces;
			const int selected_piece_index = sprite_viewer.selected_piece_index();
			const int last_piece_index = pieces.size() - 1;

			pieces.move(selected_piece_index, last_piece_index);

			sprite_viewer.setSelectedPiece(last_piece_index);
		}
	);

	connect(ui->actionMove_Piece_in_Front_of_Others, &QAction::triggered, this,
		[this]()
		{
			auto mappings = sprite_mappings_manager.lock();
			auto &pieces = mappings->frames[sprite_viewer.selected_sprite_index()].pieces;
			const int selected_piece_index = sprite_viewer.selected_piece_index();
			const int first_piece_index = 0;

			pieces.move(selected_piece_index, first_piece_index);

			sprite_viewer.setSelectedPiece(first_piece_index);
		}
	);

	//////////////////////////////////
	// Menubar: Edit/Transformation //
	//////////////////////////////////

	auto transform_frame_or_piece = [this](std::function<void(SpritePiece &piece)> callback)
	{
		auto mappings = sprite_mappings_manager.lock();
		auto &frame = mappings->frames[sprite_viewer.selected_sprite_index()];
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
	};

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

	auto move_frame_or_piece = [transform_frame_or_piece](const int x, const int y)
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

	auto set_starting_palette_line = [this](const int line)
	{
		sprite_piece_picker.setPaletteLine(line);
		sprite_viewer.setStartingPaletteLine(line);
		tile_viewer.setPaletteLine(line);
	};

	connect(ui->actionRender_Starting_with_Palette_Line_1, &QAction::triggered, this, [set_starting_palette_line](){set_starting_palette_line(0);});
	connect(ui->actionRender_Starting_with_Palette_Line_2, &QAction::triggered, this, [set_starting_palette_line](){set_starting_palette_line(1);});
	connect(ui->actionRender_Starting_with_Palette_Line_3, &QAction::triggered, this, [set_starting_palette_line](){set_starting_palette_line(2);});
	connect(ui->actionRender_Starting_with_Palette_Line_4, &QAction::triggered, this, [set_starting_palette_line](){set_starting_palette_line(3);});

	////////////////////////
	// Menubar Activation //
	////////////////////////

	auto update_menubar = [this]()
	{
		const int selected_sprite_index = sprite_viewer.selected_sprite_index();
		const bool no_sprites = sprite_mappings_manager.sprite_mappings().frames.size() == 0;
		const bool is_first_sprite = no_sprites || selected_sprite_index == 0;
		const bool is_last_sprite = no_sprites || selected_sprite_index == sprite_mappings_manager.sprite_mappings().frames.size() - 1;
		const bool no_sprite_selected = no_sprites;

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
		const bool no_pieces = no_sprites || sprite_mappings_manager.sprite_mappings().frames[selected_sprite_index].pieces.size() == 0;
		const bool is_first_piece = no_pieces || selected_piece_index == 0;
		const bool is_last_piece = no_pieces || selected_piece_index == sprite_mappings_manager.sprite_mappings().frames[selected_sprite_index].pieces.size() - 1;
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
	};

	connect(&sprite_viewer, &SpriteViewer::selectedSpriteChanged, this, update_menubar);
	connect(&sprite_mappings_manager, &SpriteMappingsManager::mappingsModified, this, update_menubar);

	// Manually update the menubar upon startup so that the various options are properly enabled.
	update_menubar();
}

MainWindow::~MainWindow()
{
	delete ui;
}
