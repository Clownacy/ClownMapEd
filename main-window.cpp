#include "main-window.h"
#include "ui_main-window.h"

#include <fstream>
#include <functional>
#include <limits>
#include <sstream>
#include <utility>

#include <QDataStream>
#include <QFile>
#include <QFileDialog>
#include <QKeyEvent>
#include <QMessageBox>
#include <QSettings>
#include <QStandardPaths>
#include <QTextStream>
#include <QtMath>

#include "CIEDE2000/CIEDE2000.h"

#include "dynamic-pattern-load-cues.h"
#include "sprite-frame.h"
#include "utilities.h"

static QString GetSlotPrefix(const int slot)
{
	return QStringLiteral("Slot") + QString::number(slot);
}

static QString GetSlotDirectory(const int slot)
{
	return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + '/' + GetSlotPrefix(slot) + '/';
}

static bool StateExists(const int slot)
{
	return QDir().exists(GetSlotDirectory(slot) + "tiles.bin");
}

static QString GameFormatToSetting(const libsonassmd::Game game)
{
	switch (game)
	{
		case libsonassmd::Game::SONIC_1:
			return "SONIC_1";

		case libsonassmd::Game::SONIC_2:
			return "SONIC_2";

		case libsonassmd::Game::SONIC_3_AND_KNUCKLES:
			return "SONIC_3_AND_KNUCKLES";
	}

	// Pick a sane default.
	return "SONIC_1";
}

static libsonassmd::Game GameFormatFromSetting(const QString &string)
{
	if (string == "SONIC_1")
		return libsonassmd::Game::SONIC_1;
	else if (string == "SONIC_2")
		return libsonassmd::Game::SONIC_2;
	else if (string == "SONIC_3_AND_KNUCKLES")
		return libsonassmd::Game::SONIC_3_AND_KNUCKLES;

	// Pick a sane default.
	return libsonassmd::Game::SONIC_1;
}

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

	QSettings settings;

	// Set-up the window's contents.
	horizontal_layout.addWidget(&sprite_piece_picker);
	horizontal_layout.addWidget(&palette_editor);
	horizontal_layout.addWidget(&sprite_viewer);

	vertical_layout.addLayout(&horizontal_layout);
//	vbox->addsp
	vertical_layout.addWidget(&tile_viewer);

	centralWidget()->setLayout(&vertical_layout);

	horizontal_layout.setContentsMargins(vertical_layout.contentsMargins());
	vertical_layout.setContentsMargins(0, 0, 0, 0);

	//////////////////////////
	// Additional Shortcuts //
	//////////////////////////

	ui->actionLoad_Backup_1->setShortcuts({QKeySequence("F1"), QKeySequence("Ctrl+L")});
	ui->actionSave_Backup_1->setShortcuts({QKeySequence("Shift+F1"), QKeySequence("Ctrl+S")});

	/* I would use 7,8/9,0 instead of 6,7/8,9, but Windows has this dumbass anti-feature that disallows Shift+0 to ever be used. */
	ui->actionPrevious_Sprite->setShortcuts({QKeySequence("8"), QKeySequence("[")});
	ui->actionNext_Sprite->setShortcuts({QKeySequence("9"), QKeySequence("]")});

	ui->actionSwap_Sprite_with_Previous->setShortcuts({QKeySequence("Ctrl+8"), QKeySequence("Ctrl+[")});
	ui->actionSwap_Sprite_with_Next->setShortcuts({QKeySequence("Ctrl+9"), QKeySequence("Ctrl+]")});

	ui->actionPrevious_Sprite_Piece->setShortcuts({QKeySequence("6"), QKeySequence("Shift+8"), QKeySequence(";"), QKeySequence("Shift+[")});
	ui->actionNext_Sprite_Piece->setShortcuts({QKeySequence("7"), QKeySequence("Shift+9"), QKeySequence("'"), QKeySequence("Shift+]")});

	ui->actionMove_Piece_into_Previous_Sprite->setShortcuts({QKeySequence("Ctrl+Shift+8"), QKeySequence("Ctrl+Shift+[")});
	ui->actionMove_Piece_into_Next_Sprite->setShortcuts({QKeySequence("Ctrl+Shift+9"), QKeySequence("Ctrl+Shift+]")});

	ui->actionMove_Piece_toward_Back->setShortcuts({QKeySequence("Ctrl+6"), QKeySequence("Ctrl+;")});
	ui->actionMove_Piece_toward_Front->setShortcuts({QKeySequence("Ctrl+7"), QKeySequence("Ctrl+'")});

	ui->actionMove_Piece_Behind_Others->setShortcuts({QKeySequence("Ctrl+Alt+Shift+6"), QKeySequence("Ctrl+Alt+Shift+;")});
	ui->actionMove_Piece_in_Front_of_Others->setShortcuts({QKeySequence("Ctrl+Alt+Shift+7"), QKeySequence("Ctrl+Alt+Shift+'")});

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
					const auto frame_index = sprite_viewer.selectedSpriteIndex();

					if (frame_index.has_value())
					{
						const auto &frames = sprite_mappings->frames;
						const auto piece_index = sprite_viewer.selectedPieceIndex();

						if (!piece_index.has_value())
						{
							if (frames[*frame_index].pieces.size() != 0)
								sprite_piece_picker.setSelectedTile(frames[*frame_index].pieces[0].tile_index);

							for (auto &piece : frames[*frame_index].pieces)
								callback(piece);
						}
						else
						{
							const auto &piece = frames[*frame_index].pieces[*piece_index];

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
	// This seems like a bit of a hack; is there any way to make the tile manager retain its selection even when there are no tiles?
	connect(&tile_manager, &TileManager::pixmapsChanged, this, tile_viewer_select_sprite);

	connect(&sprite_piece_picker, &SpritePiecePicker::pieceSelected, this,
		[this](const int width, const int height)
		{
			sprite_mappings.modify(
				[this, width, height](SpriteMappings &mappings)
				{
					if (!sprite_viewer.selectedSpriteIndex().has_value())
					{
						mappings.frames.push_back(SpriteFrame());
						sprite_viewer.setSelectedSprite(0);
					}

					auto &pieces = mappings.frames[*sprite_viewer.selectedSpriteIndex()].pieces;
					pieces.push_back({0, 0, width, height, false, 0, false, false, sprite_piece_picker.selected_tile()}); // TODO: emplace_back
					sprite_viewer.setSelectedPiece(pieces.size() - 1);
					sprite_piece_picker.setSelectedTile(sprite_piece_picker.selected_tile() + width * height);
				}
			);
		}
	);

	//////////////////
	// Window Title //
	//////////////////

	const auto update_title = [this]()
	{
		const auto do_string_thingy = [](const QString &string, const std::optional<int> index, const int total)
		{
			return " | " + string + (!index.has_value() ? "s: " : (" " + QString::number(*index, 0x10).toUpper() + "/")) + QString::number(total, 0x10).toUpper();
		};

		const QString format_string = libsonassmd::game == libsonassmd::Game::SONIC_1 ? "S1" : libsonassmd::game == libsonassmd::Game::SONIC_2 ? "S2" : "S3K";
		const QString frame_string = do_string_thingy("Frame", sprite_viewer.selectedSpriteIndex(), sprite_mappings->frames.size());
		const QString piece_string = !sprite_viewer.selectedSpriteIndex().has_value() ? "" : do_string_thingy("Piece", sprite_viewer.selectedPieceIndex(), sprite_mappings->frames[*sprite_viewer.selectedSpriteIndex()].pieces.size());
		const auto tile = tile_viewer.selection().indexOf(true);
		const QString tile_string = do_string_thingy("Tile", tile == -1 ? std::optional<int>() : tile, tile_manager.total_tiles());
		const QString dplc_string = ui->actionPattern_Load_Cues->isChecked() ? " | Cues On" : "";

		setWindowTitle(format_string + "MapEd" + frame_string + piece_string + tile_string + dplc_string);
	};

	connect(&sprite_viewer, &SpriteViewer::selectedSpriteChanged, this, update_title);
	connect(&sprite_mappings, &SignalWrapper<SpriteMappings>::modified, this, update_title);
	connect(&tile_viewer, &TileViewer::tileSelected, this, update_title);

	update_title();

	//////////////////////////////////
	// Menubar: File/Load Data File //
	//////////////////////////////////

	const auto load_tile_file = [this](std::istream &stream, const Tiles::Format format)
	{
		try
		{
			tile_manager.loadTilesFromFile(stream, format);
		}
		catch (const std::exception &e)
		{
			QMessageBox::critical(this, "Error", QStringLiteral("Failed to load file. Exception details:") + e.what());
			return;
		}

		tile_viewer.setScroll(0);
	};

	const auto load_uncompressed_tile_file = [load_tile_file](std::istream &stream)
	{
		load_tile_file(stream, Tiles::Format::BINARY);
	};

	const auto load_nemesis_tile_file = [load_tile_file](std::istream &stream)
	{
		load_tile_file(stream, Tiles::Format::NEMESIS);
	};

	const auto load_kosinski_tile_file = [load_tile_file](std::istream &stream)
	{
		load_tile_file(stream, Tiles::Format::KOSINSKI);
	};

	const auto load_moduled_kosinski_tile_file = [load_tile_file](std::istream &stream)
	{
		load_tile_file(stream, Tiles::Format::MODULED_KOSINSKI);
	};

	const auto load_kosinski_plus_tile_file = [load_tile_file](std::istream &stream)
	{
		load_tile_file(stream, Tiles::Format::KOSINSKI_PLUS);
	};

	const auto load_moduled_kosinski_plus_tile_file = [load_tile_file](std::istream &stream)
	{
		load_tile_file(stream, Tiles::Format::MODULED_KOSINSKI_PLUS);
	};

	const auto load_comper_tile_file = [load_tile_file](std::istream &stream)
	{
		load_tile_file(stream, Tiles::Format::COMPER);
	};

	const auto load_palette_file = [this](DataStream &stream, const int starting_palette_line)
	{
		palette.modify(
			[&stream, starting_palette_line](Palette &palette)
			{
				palette.fromDataStream(stream, starting_palette_line);
			}
		);
	};

	const auto is_assembly_file_path = [](const QString &file_path)
	{
		const int extension_position = file_path.lastIndexOf('.');

		return extension_position != -1 && QStringView(file_path.data() + extension_position) == QStringLiteral(".asm");
	};

	// TODO: This lambda is pointless now: get rid of it.
	const auto load_asm_or_bin_file = [](const QString &file_path, std::istream &file_stream, const std::function<void(const QString &file_path, std::istream &file_stream)> &callback)
	{
		if (file_path.isNull())
			return;

		callback(file_path, file_stream);
	};

	const auto load_sprite_mappings_file = [this, load_asm_or_bin_file, is_assembly_file_path](const QString &file_path, std::istream &file_stream)
	{
		load_asm_or_bin_file(file_path, file_stream,
			[this, is_assembly_file_path](const QString &file_path, std::istream &file_stream)
			{
				SpriteMappings new_mappings;

				try
				{
					sprite_mappings.modify(
						[&](SpriteMappings &mappings)
						{
							mappings = SpriteMappings(file_stream, is_assembly_file_path(file_path) ? SpriteMappings::Format::ASSEMBLY : SpriteMappings::Format::BINARY);;
						}
					);
				}
				catch (const std::exception &e)
				{
					QMessageBox::critical(this, "Error", QStringLiteral("Failed to load file. Exception details: ") + e.what());
					return;
				}

				sprite_viewer.setSelectedSprite(0);
			}
		);
	};

	const auto load_dynamic_pattern_load_cue_file = [this, load_asm_or_bin_file, is_assembly_file_path](const QString &file_path, std::istream &file_stream)
	{
		load_asm_or_bin_file(file_path, file_stream,
			[this, is_assembly_file_path](const QString &file_path, std::istream &file_stream)
			{
				try
				{
					DynamicPatternLoadCues dplc(file_stream, is_assembly_file_path(file_path) ? DynamicPatternLoadCues::Format::ASSEMBLY : DynamicPatternLoadCues::Format::BINARY);

					sprite_mappings.modify(
						[&dplc](SpriteMappings &mappings)
						{
							mappings = SpriteMappings(mappings, dplc);
						}
					);
				}
				catch (const std::exception &e)
				{
					QMessageBox::critical(this, "Error", QStringLiteral("Failed to load file. Exception details: ") + e.what());
					return;
				}

				// TODO: Is this needed?
				sprite_viewer.setSelectedSprite(sprite_viewer.selectedSpriteIndex());

				ui->actionPattern_Load_Cues->setChecked(true);
			}
		);
	};

	const auto load_file_std_stream = [this]([[maybe_unused]] const QString &caption, const QString &filters, const std::function<void(const QString &file_path, std::istream &file_stream)> &callback)
	{
#ifdef EMSCRIPTEN
		QFileDialog::getOpenFileContent(filters,
			[callback](const QString &file_name, const QByteArray &file_buffer)
			{
				std::stringstream file_stream(file_stream.in | file_stream.out | file_stream.binary);
				file_stream.write(file_buffer.data(), file_buffer.size());
				callback(file_name, file_stream);
			}, this
		);
#else
		const QString file_path = QFileDialog::getOpenFileName(this, caption, QString(), filters);
		std::ifstream file_stream(file_path.toStdString(), file_stream.binary);

		if (!file_stream.is_open())
		{
			QMessageBox::critical(this, "Error", "Failed to load file: file could not be opened for reading.");
			return;
		}

		callback(file_path, file_stream);
#endif
	};

	const auto load_file_data_stream = [this]([[maybe_unused]] const QString &caption, const QString &filters, const std::function<void(DataStream &stream)> &callback)
	{
#ifdef EMSCRIPTEN
		QFileDialog::getOpenFileContent(filters,
			[callback](const QString &file_name, const QByteArray &file_buffer)
			{
				DataStream file_stream(file_buffer);
				callback(file_stream);
			}, this
		);
#else
		const QString file_path = QFileDialog::getOpenFileName(this, caption, QString(), filters);

		if (file_path.isNull())
			return;

		QFile file(file_path);
		if (!file.open(QFile::ReadOnly))
		{
			QMessageBox::critical(this, "Error", "Failed to load file: file could not be opened for reading.");
			return;
		}

		DataStream stream(&file);
		callback(stream);
#endif
	};

	connect(ui->actionLoad_Tiles_Uncompressed, &QAction::triggered, this,
		[this, load_file_std_stream, load_uncompressed_tile_file]()
		{
			load_file_std_stream(
				"Open Tile Graphics File",
				"Uncompressed Tile Graphics Files (*.bin *.unc);;All Files (*.*)",
				[load_uncompressed_tile_file]([[maybe_unused]] const QString &file_path, std::istream &file_stream)
				{
					load_uncompressed_tile_file(file_stream);
				}
			);
		}
	);

	connect(ui->actionLoad_Tiles_Nemesis, &QAction::triggered, this,
		[this, load_file_std_stream, load_nemesis_tile_file]()
		{
			load_file_std_stream(
				"Open Nemesis-Compressed Tile Graphics File",
				"Nemesis-Compressed Tile Graphics Files (*.bin *.nem);;All Files (*.*)",
				[load_nemesis_tile_file]([[maybe_unused]] const QString &file_path, std::istream &file_stream)
				{
					load_nemesis_tile_file(file_stream);
				}
			);
		}
	);

	connect(ui->actionLoad_Tiles_Kosinski, &QAction::triggered, this,
		[this, load_file_std_stream, load_kosinski_tile_file]()
		{
			load_file_std_stream(
				"Open Kosinski-Compressed Tile Graphics File",
				"Kosinski-Compressed Tile Graphics Files (*.bin *.kos);;All Files (*.*)",
				[load_kosinski_tile_file]([[maybe_unused]] const QString &file_path, std::istream &file_stream)
				{
					load_kosinski_tile_file(file_stream);
				}
			);
		}
	);

	connect(ui->actionLoad_Tiles_Moduled_Kosinski, &QAction::triggered, this,
		[this, load_file_std_stream, load_moduled_kosinski_tile_file]()
		{
			load_file_std_stream(
				"Open Moduled Kosinski-Compressed Tile Graphics File",
				"Moduled Kosinski-Compressed Tile Graphics Files (*.bin *.kosm);;All Files (*.*)",
				[load_moduled_kosinski_tile_file]([[maybe_unused]] const QString &file_path, std::istream &file_stream)
				{
					load_moduled_kosinski_tile_file(file_stream);
				}
			);
		}
	);

	connect(ui->actionLoad_Tiles_Kosinski_Plus, &QAction::triggered, this,
		[this, load_file_std_stream, load_kosinski_plus_tile_file]()
		{
			load_file_std_stream(
				"Open Kosinski+-Compressed Tile Graphics File",
				"Kosinski+-Compressed Tile Graphics Files (*.bin *.kosp);;All Files (*.*)",
				[load_kosinski_plus_tile_file]([[maybe_unused]] const QString &file_path, std::istream &file_stream)
				{
					load_kosinski_plus_tile_file(file_stream);
				}
			);
		}
	);

	connect(ui->actionLoad_Tiles_Moduled_Kosinski_Plus, &QAction::triggered, this,
		[this, load_file_std_stream, load_moduled_kosinski_plus_tile_file]()
		{
			load_file_std_stream(
				"Open Moduled Kosinski+-Compressed Tile Graphics File",
				"Moduled Kosinski+-Compressed Tile Graphics Files (*.bin *.kospm);;All Files (*.*)",
				[load_moduled_kosinski_plus_tile_file]([[maybe_unused]] const QString &file_path, std::istream &file_stream)
				{
					load_moduled_kosinski_plus_tile_file(file_stream);
				}
			);
		}
	);

	connect(ui->actionLoad_Tiles_Comper, &QAction::triggered, this,
		[this, load_file_std_stream, load_comper_tile_file]()
		{
			load_file_std_stream(
				"Open Comper-Compressed Tile Graphics File",
				"Comper-Compressed Tile Graphics Files (*.bin *.comp);;All Files (*.*)",
				[load_comper_tile_file]([[maybe_unused]] const QString &file_path, std::istream &file_stream)
				{
					load_comper_tile_file(file_stream);
				}
			);
		}
	);

	connect(ui->actionLoad_Primary_Palette, &QAction::triggered, this,
		[this, load_file_data_stream, load_palette_file]()
		{
			load_file_data_stream(
				"Open Palette File",
				"Palette Files (*.bin);;All Files (*.*)",
				[load_palette_file](DataStream &stream)
				{
					load_palette_file(stream, 0);
				}
			);
		}
	);

	connect(ui->actionLoad_Secondary_Palette_Lines, &QAction::triggered, this,
		[this, load_file_data_stream, load_palette_file]()
		{
			load_file_data_stream(
				"Open Palette File",
				"Palette Files (*.bin);;All Files (*.*)",
				[load_palette_file](DataStream &stream)
				{
					load_palette_file(stream, 1);
				}
			);
		}
	);

	connect(ui->actionLoad_Mappings, &QAction::triggered, this,
		[this, load_file_std_stream, load_sprite_mappings_file]()
		{
			load_file_std_stream(
				"Open Sprite Mappings File",
				"Sprite Mapping Files (*.bin *.asm);;All Files (*.*)",
				load_sprite_mappings_file
			);
		}
	);

	connect(ui->actionLoad_Sprite_Pattern_Cues, &QAction::triggered, this,
		[this, load_file_std_stream, load_dynamic_pattern_load_cue_file]()
		{
			// TODO: Filters.
			load_file_std_stream(
				"Open Dynamic Pattern Loading Cue File",
				"Pattern Cue Files (*.bin *.asm);;All Files (*.*)",
				load_dynamic_pattern_load_cue_file
			);
		}
	);

	connect(ui->actionExit, &QAction::triggered, this, &QMainWindow::close);

	//////////////////////////////////
	// Menubar: File/Save Data File //
	//////////////////////////////////

	[[maybe_unused]] const auto ask_if_assembly = [this]()
	{
		return QMessageBox::question(this, "Format", "Save as assembly instead of binary?") == QMessageBox::Yes;
	};

	const auto save_file_std_stream = [this, ask_if_assembly, is_assembly_file_path]([[maybe_unused]] QString hint_filename, const QString &caption, const QString &filters, const std::function<void(std::ostream &file_stream, bool is_assembly_file)> &callback, [[maybe_unused]] const bool should_ask_if_assembly = false, std::ios::openmode flags = {})
	{
#ifdef EMSCRIPTEN
		const bool is_assembly_file = should_ask_if_assembly && ask_if_assembly();

		if (is_assembly_file)
		{
			const auto dot = hint_filename.lastIndexOf('.');

			if (dot != -1)
				hint_filename = hint_filename.first(dot) + ".asm";
		}

		std::stringstream stream(flags | std::ios::in | std::ios::out);
		callback(stream, is_assembly_file);
		const auto stream_string = stream.str();
		QFileDialog::saveFileContent(QByteArray(stream_string.data(), stream_string.size()), hint_filename, this);
#else
		const QString file_path = QFileDialog::getSaveFileName(this, caption, QString(), filters, nullptr, QFileDialog::DontConfirmOverwrite);

		if (file_path.isNull())
			return;

		const bool is_assembly_file = is_assembly_file_path(file_path);

		if (is_assembly_file)
			flags &= ~std::ios::binary;

		std::ofstream stream(file_path.toStdString(), flags);

		if (!stream.is_open())
		{
			QMessageBox::critical(this, "Error", "Failed to save file: file could not be opened for writing.");
			return;
		}

		callback(stream, is_assembly_file);
#endif
	};

	const auto save_file_data_stream = [this, ask_if_assembly, is_assembly_file_path]([[maybe_unused]] const QString &hint_filename, const QString &caption, const QString &filters, const std::function<void(DataStream &file_stream, bool is_assembly_file)> &callback, [[maybe_unused]] const bool should_ask_if_assembly = false)
	{
#ifdef EMSCRIPTEN
		const bool is_assembly_file = should_ask_if_assembly && ask_if_assembly();

		QByteArray buffer;
		DataStream stream(&buffer, QIODeviceBase::ReadWrite);
		callback(stream, is_assembly_file);
		QFileDialog::saveFileContent(buffer, hint_filename, this);
#else
		const QString file_path = QFileDialog::getSaveFileName(this, caption, QString(), filters, nullptr, QFileDialog::DontConfirmOverwrite);

		if (file_path.isNull())
			return;

		const bool is_assembly_file = is_assembly_file_path(file_path);

		QFile::OpenMode flags = QFile::OpenModeFlag::WriteOnly;

		if (is_assembly_file)
			flags.setFlag(QFile::OpenModeFlag::Text);

		QFile file(file_path);
		if (!file.open(flags))
			QMessageBox::critical(this, "Error", "Failed to save file: file could not be opened for writing.");

		DataStream stream(&file);

		callback(stream, is_assembly_file);
#endif
	};

	const auto save_tile_file = [this, save_file_std_stream](const QString &compression_name, const QString &file_extension, const Tiles::Format format)
	{
		save_file_std_stream("tiles." + file_extension, "Save " + compression_name + " Tile Graphics File", compression_name + " Tile Graphics Files (*." + file_extension + " *.bin *.asm);;All Files (*.*)",
			[&](std::ostream &file_stream, const bool is_assembly_file)
			{
				try
				{
					const auto &tiles = tile_manager.getTiles();

					if (is_assembly_file)
					{
						std::stringstream string_stream(std::ios::in | std::ios::out | std::ios::binary);
						string_stream.exceptions(std::ios::badbit | std::ios::eofbit | std::ios::failbit);

						tiles.toStream(string_stream, format);

						file_stream << QStringLiteral("; --------------------------------------------------------------------------------\n"
						                              "; %1 tile graphics - output from ClownMapEd\n"
						                              "; --------------------------------------------------------------------------------\n"
						                              ).arg(compression_name).toStdString();

						const std::string::size_type bytes_per_line = 0x20;
						const std::string &output_string = string_stream.str();
						for (std::string::size_type i = 0; i < output_string.size(); i += bytes_per_line)
						{
							file_stream << "\tdc.b ";

							for (std::string::size_type j = 0; j < qMin(bytes_per_line, output_string.size() - i); ++j)
							{
								if (j != 0)
									file_stream << ',';

								file_stream << '$' << Utilities::IntegerToZeroPaddedHexQString(static_cast<quint8>(output_string[i + j])).toStdString();
							}

							file_stream << "; " << i << '\n';
						}

						file_stream << "\teven\n";
					}
					else
					{
						tiles.toStream(file_stream, format);
					}
				}
				catch (const std::exception &e)
				{
					QMessageBox::critical(this, "Error", QStringLiteral("Failed to save file. Exception details: ") + e.what());
				}
			}, true, format == Tiles::Format::ASSEMBLY ? std::ios::openmode() : std::ios::binary
		);
	};

	connect(ui->actionUncompressed, &QAction::triggered, this,
		[save_tile_file]()
		{
			save_tile_file("Uncompressed", "unc", Tiles::Format::BINARY);
		}
	);

	connect(ui->actionNemesis, &QAction::triggered, this,
		[save_tile_file]()
		{
			save_tile_file("Nemesis-Compressed", "nem", Tiles::Format::NEMESIS);
		}
	);

	connect(ui->actionKosinski, &QAction::triggered, this,
		[save_tile_file]()
		{
			save_tile_file("Kosinski-Compressed", "kos", Tiles::Format::KOSINSKI);
		}
	);

	connect(ui->actionModuled_Kosinski, &QAction::triggered, this,
		[this, save_tile_file]()
		{
			if (tile_manager.getTiles().total_bytes() > 0xFFFF)
			{
				QMessageBox::critical(this, "Error", "Tile data is too large for moduled compression.");
				return;
			}

			save_tile_file("Moduled Kosinski-Compressed", "kosm", Tiles::Format::MODULED_KOSINSKI);
		}
	);

	connect(ui->actionKosinski_2, &QAction::triggered, this,
		[save_tile_file]()
		{
			save_tile_file("Kosinski+-Compressed", "kosp", Tiles::Format::KOSINSKI_PLUS);
		}
	);

	connect(ui->actionModuled_Kosinski_2, &QAction::triggered, this,
		[this, save_tile_file]()
		{
			if (tile_manager.getTiles().total_bytes() > 0xFFFF)
			{
				QMessageBox::critical(this, "Error", "Tile data is too large for moduled compression.");
				return;
			}

			save_tile_file("Moduled Kosinski+-Compressed", "kospm", Tiles::Format::MODULED_KOSINSKI_PLUS);
		}
	);

	connect(ui->actionComper, &QAction::triggered, this,
		[save_tile_file]()
		{
			save_tile_file("Comper-Compressed", "comp", Tiles::Format::COMPER);
		}
	);

	const auto save_palette_file = [this, save_file_data_stream](const int starting_palette_line, const int ending_palette_line)
	{
		save_file_data_stream("palette.bin", "Save Palette File", "Palette Files (*.bin);;All Files (*.*)",
			[&](DataStream &file_stream, [[maybe_unused]] const bool is_assembly_file)
			{
				palette->toDataStream(file_stream, starting_palette_line, ending_palette_line);
			}
		);
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

	// TODO: This lambda is pointless now: get rid of it.
	const auto save_asm_or_bin_file = [this, save_file_std_stream]([[maybe_unused]] const QString &hint_filename, const QString &caption, const QString &filters, const std::function<void(std::ostream &stream, bool saving_assembly_file)> &callback)
	{
		save_file_std_stream(hint_filename, caption, filters,
			[&](std::ostream &file_stream, const bool is_assembly_file)
			{
				try
				{
					callback(file_stream, is_assembly_file);
				}
				catch (const std::exception &e)
				{
					QMessageBox::critical(this, "Error", QStringLiteral("Failed to save file. Exception details: ") + e.what());
				}
			}, true, std::ios::binary
		);
	};

	connect(ui->actionSave_Mappings, &QAction::triggered, this,
		[this, save_asm_or_bin_file]()
		{
			save_asm_or_bin_file("mappings.bin", "Save Sprite Mappings File", "Sprite Mapping Files (*.bin *.asm);;All Files (*.*)",
				[this](std::ostream &stream, const bool saving_assembly_file)
				{
					const bool mapmacros = !ui->actionLegacyFormats->isChecked();

					auto sprite_mappings_copy = *sprite_mappings;

					if (ui->actionPattern_Load_Cues->isChecked())
						sprite_mappings_copy.removeDPLCs();

					if (saving_assembly_file)
					{
						stream << QStringLiteral("; --------------------------------------------------------------------------------\n"
						                         "; Sprite mappings - output from ClownMapEd - %1 format\n"
						                         "; --------------------------------------------------------------------------------\n\n"
						                        ).arg(mapmacros ? QStringLiteral("MapMacros") : libsonassmd::game == libsonassmd::Game::SONIC_1 ? QStringLiteral("Sonic 1/CD") : libsonassmd::game == libsonassmd::Game::SONIC_2 ? QStringLiteral("Sonic 2") : QStringLiteral("Sonic 3 & Knuckles")).toStdString();

						sprite_mappings_copy.toStream(stream, SpriteMappings::Format::ASSEMBLY);
					}
					else
					{
						sprite_mappings_copy.toStream(stream, SpriteMappings::Format::BINARY);
					}
				}
			);
		}
	);

	connect(ui->actionSave_Pattern_Cues, &QAction::triggered, this,
		[this, save_asm_or_bin_file]()
		{
			save_asm_or_bin_file("dplc.bin", "Save Dynamic Pattern Loading Cue File", "Pattern Cue Files (*.bin *.asm);;All Files (*.*)",
				[this](std::ostream &stream, const bool saving_assembly_file)
				{
					const bool mapmacros = !ui->actionLegacyFormats->isChecked();

					auto sprite_mappings_copy = *sprite_mappings;
					const auto dplc = sprite_mappings_copy.removeDPLCs();

					if (saving_assembly_file)
					{
						stream << QStringLiteral("; --------------------------------------------------------------------------------\n"
						                         "; Dynamic Pattern Loading Cues - output from ClownMapEd - %1 format\n"
						                         "; --------------------------------------------------------------------------------\n\n"
						                        ).arg(mapmacros ? QStringLiteral("MapMacros") : libsonassmd::game == libsonassmd::Game::SONIC_1 ? QStringLiteral("Sonic 1") : QStringLiteral("Sonic 2/3&K/CD")).toStdString();

						dplc.toStream(stream, DynamicPatternLoadCues::Format::ASSEMBLY);
					}
					else
					{
						dplc.toStream(stream, DynamicPatternLoadCues::Format::BINARY);
					}
				}
			);
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

			// TODO: Is this line needed?
			sprite_viewer.setSelectedSprite(sprite_viewer.selectedSpriteIndex());
			ui->actionPattern_Load_Cues->setChecked(false);
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
	// Menubar: File/Load from Image //
	///////////////////////////////////

	const auto frame_to_qimage = [this](const SpriteFrame &frame, const bool render_as_is)
	{
		const auto &frame_rect = calculateRect(frame);

		QImage image(frame_rect.width(), frame_rect.height(), QImage::Format_RGB32);

		// Fill image with a reserved colour (this indicates
		// transparency and should never occur in a sprite).
		image.fill(QColor(0xFF, 0, 0xFF));

		// Render the sprite onto the image.
		QPainter painter(&image);

		draw(frame, painter, !render_as_is, tile_manager, render_as_is ? TileManager::PixmapType::NO_BACKGROUND : TileManager::PixmapType::WITH_BACKGROUND, sprite_viewer.startingPaletteLine(), -frame_rect.left(), -frame_rect.top());

		return image;
	};

	connect(ui->actionImport_Sprite_over_Active_Frame, &QAction::triggered, this,
		[this, frame_to_qimage]()
		{
			const auto load_image_from_file = [this](const QString &caption, const QString &filters, const std::function<void(QImage &image)> &callback)
			{
#ifdef EMSCRIPTEN
				QFileDialog::getOpenFileContent(filters,
					[callback]([[maybe_unused]] const QString &file_path, const QByteArray &file_buffer)
					{
						QImage image;
						image.loadFromData(file_buffer);
						callback(image);
					}, this
				);
#else
				QImage image;

				const QString file_path = QFileDialog::getOpenFileName(this, caption, QString(), filters);

				if (!file_path.isNull())
					image.load(file_path);

				callback(image);
#endif
			};

			load_image_from_file("Import Sprite Frame", "Image (*.png *.bmp)",
				[this, frame_to_qimage](QImage &their_image)
				{
					if (their_image.isNull())
					{
						QMessageBox::critical(this, "Error", "Failed to import image: file could not be loaded.");
						return;
					}

					const SpriteFrame &frame = sprite_mappings->frames[sprite_viewer.selectedSpriteIndex().value()];
					const QImage our_image = frame_to_qimage(frame, false);

					if (their_image.width() != our_image.width() || their_image.height() != our_image.height())
					{
						// TODO: Use an error message that is similar to SonMapEd's.
						QMessageBox::critical(this, "Error", QStringLiteral("Failed to import image: the imported image (%1x%2) is a different size to the sprite that it is replacing (%3x%4).").arg(QString::number(their_image.width()), QString::number(their_image.height()), QString::number(our_image.width()), QString::number(our_image.height())));
						return;
					}

					// Finally, import the image over the sprite.
					tile_manager.modifyTiles(
						[this, &frame, &their_image](std::vector<libsonassmd::Tile> &old_tiles)
						{
							// TODO: This doesn't account for overlapping pieces, which is a mistake that SonMapEd also makes. Is there anything that can be done about this?
							const QVector<SpritePiece::Tile> new_tiles = getUniqueTiles(frame);
							const int frame_left = calculateRect(frame).left();
							const int frame_top = calculateRect(frame).top();

							// Overwrite each unique tile.
							for (const auto &tile : new_tiles)
							{
								for (int y = 0; y < TileManager::TILE_HEIGHT; ++y)
								{
									const int image_y = tile.y + (tile.y_flip ? TileManager::TILE_HEIGHT - y - 1 : y) - frame_top;

									for (int x = 0; x < TileManager::TILE_HEIGHT; ++x)
									{
										const int image_x = tile.x + (tile.x_flip ? TileManager::TILE_WIDTH - x - 1 : x) - frame_left;

										// This big block of code is responsible for converting a single pixel.

										const auto their_colour = Utilities::QColorToLAB(QColor(their_image.pixel(image_x, image_y)));

										// Find the closest colour in the palette line to the imported pixel colour.
										uint closest_colour_index = 0;
										double closest_distance = std::numeric_limits<double>::max();

										for (uint colour_index = 0; colour_index < Palette::COLOURS_PER_LINE; ++colour_index)
										{
											const auto our_colour = Utilities::QColorToLAB(palette->lines[(tile.palette_line + sprite_viewer.startingPaletteLine()) % Palette::TOTAL_LINES].colours[colour_index].toQColor224());

											CIEDE2000::LAB lab1 = {their_colour[0], their_colour[1], their_colour[2]};
											CIEDE2000::LAB lab2 = {our_colour[0], our_colour[1], our_colour[2]};
											const double distance = CIEDE2000::CIEDE2000(lab1, lab2);

											// Only allow matching the transparency colour if it is exact.
											if ((colour_index != 0 && distance < closest_distance) || their_colour == our_colour)
											{
												closest_distance = distance;
												closest_colour_index = colour_index;
											}
										}

										// The sprite might be using out-of-bound tiles, so create room for the new tiles if we have to.
										if (tile.index >= old_tiles.size())
											old_tiles.resize(tile.index + 1);

										old_tiles[tile.index].pixels[y][x] = closest_colour_index;
									}
								}
							}
						}
					);
				}
			);
		}
	);

	/////////////////////////////////
	// Menubar: File/Save to Image //
	/////////////////////////////////

	const auto export_image = [this, save_file_data_stream](const bool render_as_is)
	{
		const auto &frame = sprite_mappings->frames[sprite_viewer.selectedSpriteIndex().value()];
		const auto &frame_rect = calculateRect(frame);

		QImage image(frame_rect.width(), frame_rect.height(), QImage::Format_RGB32);

		// Fill image with a reserved colour (this indicates
		// transparency and should never occur in a sprite).
		image.fill(QColor(0xFF, 0, 0xFF));

		// Render the sprite onto the image.
		QPainter painter(&image);

		draw(frame, painter, !render_as_is, tile_manager, render_as_is ? TileManager::PixmapType::NO_BACKGROUND : TileManager::PixmapType::WITH_BACKGROUND, sprite_viewer.startingPaletteLine(), -frame_rect.left(), -frame_rect.top());

		const auto save_image = [&](const QImage &image, const QString &caption, const QString &filters)
		{
#ifdef EMSCRIPTEN
			bool success = false;

			save_file_data_stream("image.png", caption, filters,
				[&](DataStream &file_stream, [[maybe_unused]] const bool is_assembly_file)
				{
					success = image.save(file_stream.device(), "PNG");
				}
			);

			return success;
#else
			const QString file_path = QFileDialog::getSaveFileName(this, caption, QString(), filters);

			if (file_path.isNull())
				return true;

			return image.save(file_path);
#endif
		};

		if (!save_image(image, render_as_is ? "Render Sprite Frame" : "Export Sprite Frame", "Image (*.png *.bmp)"))
			QMessageBox::critical(this, "Error", "Failed to export image.");
	};

	connect(ui->actionExport_Sprite_Frame, &QAction::triggered, this,
		[export_image]()
		{
			export_image(false);
		}
	);

	connect(ui->actionRender_Sprite_Image, &QAction::triggered, this,
		[export_image]()
		{
			export_image(true);
		}
	);

	//////////////////////////////
	// Menubar: File/Load State //
	//////////////////////////////

#define DO_LOAD_STATE_BUTTON(NUMBER) \
	connect(ui->actionLoad_Backup_##NUMBER, &QAction::triggered, this, \
		[this]() \
		{ \
			LoadState(NUMBER); \
		} \
	); \
	ui->actionLoad_Backup_##NUMBER->setDisabled(!StateExists(NUMBER));

	DO_LOAD_STATE_BUTTON(1);
	DO_LOAD_STATE_BUTTON(2);
	DO_LOAD_STATE_BUTTON(3);
	DO_LOAD_STATE_BUTTON(4);
	DO_LOAD_STATE_BUTTON(5);
	DO_LOAD_STATE_BUTTON(6);
	DO_LOAD_STATE_BUTTON(7);
	DO_LOAD_STATE_BUTTON(8);
	DO_LOAD_STATE_BUTTON(9);
#undef DO_LOAD_STATE_BUTTON

	//////////////////////////////
	// Menubar: File/Save State //
	//////////////////////////////

#define DO_SAVE_STATE_BUTTON(NUMBER) \
	connect(ui->actionSave_Backup_##NUMBER, &QAction::triggered, this, \
		[this]() \
		{ \
			SaveState(NUMBER); \
		} \
	);
	DO_SAVE_STATE_BUTTON(1);
	DO_SAVE_STATE_BUTTON(2);
	DO_SAVE_STATE_BUTTON(3);
	DO_SAVE_STATE_BUTTON(4);
	DO_SAVE_STATE_BUTTON(5);
	DO_SAVE_STATE_BUTTON(6);
	DO_SAVE_STATE_BUTTON(7);
	DO_SAVE_STATE_BUTTON(8);
	DO_SAVE_STATE_BUTTON(9);
#undef DO_SAVE_STATE_BUTTON

	//////////////////////////
	// Menubar: Edit/Sprite //
	//////////////////////////

	connect(ui->actionNext_Sprite, &QAction::triggered, this, [this](){sprite_viewer.setSelectedSprite(sprite_viewer.selectedSpriteIndex().value() + 1);});
	connect(ui->actionPrevious_Sprite, &QAction::triggered, this, [this](){sprite_viewer.setSelectedSprite(sprite_viewer.selectedSpriteIndex().value() - 1);});

	connect(ui->actionSwap_Sprite_with_Next, &QAction::triggered, this,
		[this]()
		{
			sprite_mappings.modify(
				[this](SpriteMappings &mappings)
				{
					auto &frames = mappings.frames;
					const int selected_sprite_index = sprite_viewer.selectedSpriteIndex().value();

					Utilities::Move(frames, selected_sprite_index, selected_sprite_index + 1);
					sprite_viewer.setSelectedSprite(selected_sprite_index + 1);
				}
			);

			sprite_viewer.setSelectedSprite(sprite_viewer.selectedSpriteIndex());
	}
	);

	connect(ui->actionSwap_Sprite_with_Previous, &QAction::triggered, this,
		[this]()
		{
			sprite_mappings.modify(
				[this](SpriteMappings &mappings)
				{
					auto &frames = mappings.frames;
					const int selected_sprite_index = sprite_viewer.selectedSpriteIndex().value();

					Utilities::Move(frames, selected_sprite_index, selected_sprite_index - 1);
					sprite_viewer.setSelectedSprite(selected_sprite_index - 1);
				}
			);

			sprite_viewer.setSelectedSprite(sprite_viewer.selectedSpriteIndex());
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
					mappings.frames.emplace(mappings.frames.cbegin() + (sprite_viewer.selectedSpriteIndex().value_or(-1) + 1));
				}
			);

			sprite_viewer.setSelectedSprite(sprite_viewer.selectedSpriteIndex().value() + 1);
		}
	);

	connect(ui->actionDuplicate_Sprite, &QAction::triggered, this,
		[this]()
		{
			sprite_mappings.modify(
				[this](SpriteMappings &mappings)
				{
					auto &frames = mappings.frames;
					const int selected_sprite_index = sprite_viewer.selectedSpriteIndex().value();

					frames.insert(frames.cbegin() + selected_sprite_index + 1, frames[selected_sprite_index]);
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
					mappings.frames.erase(mappings.frames.cbegin() + sprite_viewer.selectedSpriteIndex().value());
				}
			);

			sprite_viewer.setSelectedSprite(sprite_viewer.selectedSpriteIndex());
		}
	);

	/////////////////////////
	// Menubar: Edit/Piece //
	/////////////////////////

	connect(ui->actionNext_Sprite_Piece, &QAction::triggered, this,
		[this]()
		{
			sprite_viewer.selectNextPiece();
		}
	);

	connect(ui->actionPrevious_Sprite_Piece, &QAction::triggered, this,
		[this]()
		{
			sprite_viewer.selectPreviousPiece();
		}
	);

	connect(ui->actionDelete_Sprite_Piece, &QAction::triggered, this,
		[this]()
		{
			const int piece_to_delete = sprite_viewer.selectedPieceIndex().value();
			sprite_viewer.selectPreviousPiece();

			sprite_mappings.modify(
				[this, piece_to_delete](SpriteMappings &mappings)
				{
					auto &pieces = mappings.frames[sprite_viewer.selectedSpriteIndex().value()].pieces;

					pieces.erase(pieces.cbegin() + piece_to_delete);
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
					auto &pieces = mappings.frames[sprite_viewer.selectedSpriteIndex().value()].pieces;
					const int selected_piece_index = sprite_viewer.selectedPieceIndex().value();

					pieces.insert(pieces.cbegin() + selected_piece_index + 1, pieces[selected_piece_index]);
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
					const int selected_sprite_index = sprite_viewer.selectedSpriteIndex().value();
					const int next_sprite_index = selected_sprite_index + 1;
					const int selected_piece_index = sprite_viewer.selectedPieceIndex().value();
					auto &frames = mappings.frames;
					auto &pieces = frames[selected_sprite_index].pieces;

					frames[next_sprite_index].pieces.push_back(pieces[selected_piece_index]);
					pieces.erase(pieces.cbegin() + selected_piece_index);

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
					const int selected_sprite_index = sprite_viewer.selectedSpriteIndex().value();
					const int previous_sprite_index = selected_sprite_index - 1;
					const int selected_piece_index = sprite_viewer.selectedPieceIndex().value();
					auto &frames = mappings.frames;
					auto &pieces = frames[selected_sprite_index].pieces;

					frames[previous_sprite_index].pieces.push_back(pieces[selected_piece_index]);
					pieces.erase(pieces.cbegin() + selected_piece_index);

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
					const int selected_sprite_index = sprite_viewer.selectedSpriteIndex().value();
					const int next_sprite_index = selected_sprite_index + 1;
					const int selected_piece_index = sprite_viewer.selectedPieceIndex().value();
					auto &frames = mappings.frames;
					auto &pieces = frames[selected_sprite_index].pieces;

					frames.emplace(frames.cbegin() + next_sprite_index);
					frames[next_sprite_index].pieces.push_back(pieces[selected_piece_index]);
					pieces.erase(pieces.cbegin() + selected_piece_index);

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
					auto &pieces = mappings.frames[sprite_viewer.selectedSpriteIndex().value()].pieces;
					const int selected_piece_index = sprite_viewer.selectedPieceIndex().value();
					const int next_piece_index = selected_piece_index + 1;

					Utilities::Move(pieces, selected_piece_index, next_piece_index);

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
					auto &pieces = mappings.frames[sprite_viewer.selectedSpriteIndex().value()].pieces;
					const int selected_piece_index = sprite_viewer.selectedPieceIndex().value();
					const int previous_piece_index = selected_piece_index - 1;

					Utilities::Move(pieces, selected_piece_index, previous_piece_index);

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
					auto &pieces = mappings.frames[sprite_viewer.selectedSpriteIndex().value()].pieces;
					const int selected_piece_index = sprite_viewer.selectedPieceIndex().value();
					const int last_piece_index = pieces.size() - 1;

					Utilities::Move(pieces, selected_piece_index, last_piece_index);

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
					auto &pieces = mappings.frames[sprite_viewer.selectedSpriteIndex().value()].pieces;
					const int selected_piece_index = sprite_viewer.selectedPieceIndex().value();
					const int first_piece_index = 0;

					Utilities::Move(pieces, selected_piece_index, first_piece_index);

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
				auto &frame = mappings.frames[sprite_viewer.selectedSpriteIndex().value()];
				const auto piece_index = sprite_viewer.selectedPieceIndex();

				if (piece_index.has_value())
				{
					auto &piece = frame.pieces[*piece_index];
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
			const auto &frame = sprite_mappings->frames[sprite_viewer.selectedSpriteIndex().value()];
			const QRect &rect = !sprite_viewer.selectedPieceIndex().has_value() ? calculateRect(frame) : calculateRect(frame.pieces[sprite_viewer.selectedPieceIndex().value()]);

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
			const auto &frame = sprite_mappings->frames[sprite_viewer.selectedSpriteIndex().value()];
			const QRect &rect = !sprite_viewer.selectedPieceIndex().has_value() ? calculateRect(frame) : calculateRect(frame.pieces[sprite_viewer.selectedPieceIndex().value()]);

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

	/////////////////////////
	// Menubar: Edit/Tiles //
	/////////////////////////

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

					for (const auto &frame : std::as_const(sprite_mappings->frames))
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
			// Cache this, because various signals will cause it to be changed later.
			const QVector<bool> selection = tile_viewer.selection();

			tile_manager.modifyTiles(
				[&](Tiles &tiles)
				{
					Tiles new_tiles;
					new_tiles.reserve(selection.size());

					sprite_mappings.modify(
						[&](SpriteMappings &mappings)
						{
							for (int i = 0; i < selection.size(); ++i)
							{
								if (selection[i])
								{
									// Correct sprite piece tile indices to account for the removed tile.
									for (auto &frame : mappings.frames)
										for (auto &piece : frame.pieces)
											if (piece.tile_index > new_tiles.size())
												--piece.tile_index;
								}
								else
								{
									new_tiles.push_back(tiles[i]);
								}
							}
						}
					);

					tiles = std::move(new_tiles);
				}
			);

			// TODO: Is this necessary?
			sprite_viewer.setSelectedSprite(sprite_viewer.selectedSpriteIndex());
		}
	);

	connect(ui->actionFind_Sprite_that_Uses_Focused_Tile, &QAction::triggered, this,
		[this]()
		{
			const int selected_tile_index = tile_viewer.selection().indexOf(true);

			const auto &frames = sprite_mappings->frames;
			for (int frame_index = 0; frame_index < static_cast<int>(frames.size()); ++frame_index)
			{
				const auto &frame = frames[frame_index];

				// TODO: Improve this.
				// This algorithm sucks, but it's exactly what SonMapEd does.
				// The problem with this is that it only toggles between the first
				// two sprites to use the selected tile, when it would be better if
				// it cycled between every sprite that uses the tile instead.
				if (frame_index != sprite_viewer.selectedSpriteIndex())
				{
					for (uint piece_index = 0; piece_index < frame.pieces.size(); ++piece_index)
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
							int selected_tile = 0;
							while ((selected_tile = selected.lastIndexOf(true, selected_tile - 1)) != -1)
							{
								// Duplicate the tile.
								int insertion_index = selected.indexOf(false, selected_tile);

								if (insertion_index == -1)
									insertion_index = selected.size();

								tile_manager.duplicateTile(selected_tile, insertion_index);

								// Correct sprite piece tile indices to account for the inserted tile.
								for (auto &frame : mappings.frames)
									for (auto &piece : frame.pieces)
										if (piece.tile_index >= insertion_index)
											++piece.tile_index;

								if (selected_tile == 0)
									break;
							}
						}
					);

					// TODO: Is this necessary?
					sprite_viewer.setSelectedSprite(sprite_viewer.selectedSpriteIndex());
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
					for (const auto &piece : std::as_const(sprite_mappings->frames[sprite_viewer.selectedSpriteIndex().value()].pieces))
						for (int i = 0; i < piece.width * piece.height; ++i)
							selected[piece.tile_index + i] = true;
				}
			);
		}
	);

	//////////////////////////
	// Menubar: View/Scroll //
	//////////////////////////

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

	///////////////////////////////////
	// Menubar: Settings/Game Format //
	///////////////////////////////////

	const auto set_game_format = [this, update_title](const libsonassmd::Game game)
	{
		libsonassmd::game = game;

		ui->actionSonic_1->setChecked(game == libsonassmd::Game::SONIC_1);
		ui->actionSonic_2->setChecked(game == libsonassmd::Game::SONIC_2);
		ui->actionSonic_3_Knuckles->setChecked(game == libsonassmd::Game::SONIC_3_AND_KNUCKLES);

		update_title();
	};

	connect(ui->actionSonic_1, &QAction::triggered, this, [set_game_format](){set_game_format(libsonassmd::Game::SONIC_1);});
	connect(ui->actionSonic_2, &QAction::triggered, this, [set_game_format](){set_game_format(libsonassmd::Game::SONIC_2);});
	connect(ui->actionSonic_3_Knuckles, &QAction::triggered, this, [set_game_format](){set_game_format(libsonassmd::Game::SONIC_3_AND_KNUCKLES);});

	set_game_format(GameFormatFromSetting(settings.value("GameFormat", "SONIC_1").toString()));

	//////////////////////////////////////
	// Menubar: Settings/Tile Rendering //
	//////////////////////////////////////

	connect(ui->actionOrient_Tiles_Vertically, &QAction::toggled, &tile_viewer, &TileViewer::setVerticalOrientation);
	ui->actionOrient_Tiles_Vertically->setChecked(settings.value("OrientTilesVertically", false).toBool());

	connect(ui->actionHide_Duplicated_Tiles_in_Frames, &QAction::toggled, &sprite_viewer, &SpriteViewer::setHideDuplicateTiles);
	ui->actionHide_Duplicated_Tiles_in_Frames->setChecked(settings.value("HideDuplicatedTilesInFrames", false).toBool());

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

	set_starting_palette_line(settings.value("StartingPaletteLine", 0).toInt());

	///////////////////////
	// Menubar: Settings //
	///////////////////////

	const auto &SetUpCheckBox = [this, &settings](QAction* const action, const std::function<void(bool)> &callback, const QString &setting_key)
	{
		connect(action, &QAction::toggled, this, callback);
		const bool setting_value = settings.value(setting_key, false).toBool();
		action->setChecked(setting_value);
		callback(setting_value);
	};

	const auto set_pattern_load_cues_enabled = [this, update_title](const bool enabled)
	{
		ui->actionSave_Pattern_Cues->setEnabled(enabled);
		ui->actionUnload_Pattern_Cues->setEnabled(enabled);
		update_title();
	};

	SetUpCheckBox(ui->actionPattern_Load_Cues, set_pattern_load_cues_enabled, "PatternLoadCues");

	const auto set_legacy_assembly_formats_enabled = [](const bool enabled)
	{
		libsonassmd::mapmacros = !enabled;
	};

	SetUpCheckBox(ui->actionLegacyFormats, set_legacy_assembly_formats_enabled, "LegacyAssemblyFormats");

	///////////////////
	// Menubar: Help //
	///////////////////

	connect(ui->actionAbout, &QAction::triggered, &about, &QDialog::show);
	connect(ui->actionLicences, &QAction::triggered, &licences, &QDialog::show);

	/////////////////////////////
	// Load the cached assets. //
	/////////////////////////////

	LoadState(0);

	////////////////////////
	// Menubar Activation //
	////////////////////////

	const auto update_menubar = [this]()
	{
		const auto selected_sprite_index = sprite_viewer.selectedSpriteIndex();
		const bool no_sprites = sprite_mappings->frames.size() == 0;
		const bool is_first_sprite = no_sprites || selected_sprite_index == 0;
		const bool is_last_sprite = no_sprites || selected_sprite_index == static_cast<int>(sprite_mappings->frames.size() - 1);
		const bool no_sprite_selected = no_sprites;

		ui->actionExport_Sprite_Frame->setDisabled(no_sprite_selected);
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

		const auto selected_piece_index = sprite_viewer.selectedPieceIndex();
		const bool no_pieces = no_sprites || !selected_sprite_index.has_value() || sprite_mappings->frames[*selected_sprite_index].pieces.size() == 0;
		const bool is_first_piece = no_pieces || selected_piece_index == 0;
		const bool is_last_piece = no_pieces || selected_piece_index == static_cast<int>(sprite_mappings->frames[*selected_sprite_index].pieces.size() - 1);
		const bool no_piece_selected = no_pieces || !selected_piece_index.has_value();

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

#ifdef EMSCRIPTEN
	// We don't need the Exit button on the web.
	ui->actionExit->deleteLater();
#endif

	// Manually update the menubar upon startup so that the various options are properly enabled.
	update_menubar();
}

MainWindow::~MainWindow()
{
	// Save settings.
	QSettings settings;
	settings.setValue("GameFormat", GameFormatToSetting(libsonassmd::game));
	settings.setValue("OrientTilesVertically", ui->actionOrient_Tiles_Vertically->isChecked());
	settings.setValue("HideDuplicatedTilesInFrames", ui->actionHide_Duplicated_Tiles_in_Frames->isChecked());
	settings.setValue("StartingPaletteLine", tile_viewer.paletteLine());
	settings.setValue("PatternLoadCues", ui->actionPattern_Load_Cues->isChecked());
	settings.setValue("LegacyAssemblyFormats", ui->actionLegacyFormats->isChecked());

	SaveState(0);

	delete ui;
}

void MainWindow::SaveState(const int slot)
{
	QSettings settings;
	const QString prefix = GetSlotPrefix(slot);
	settings.setValue(prefix + "SelectedSpriteIndex", sprite_viewer.selectedSpriteIndex().value_or(-1));
	settings.setValue(prefix + "SelectedPieceIndex", sprite_viewer.selectedPieceIndex().value_or(-1));

	// Save cached assets.
	const QString directory = GetSlotDirectory(slot);
	QDir().mkpath(directory);
	QFile file(directory + "palette.bin");
	if (file.open(QFile::OpenModeFlag::WriteOnly))
	{
		DataStream stream(&file);
		palette->toDataStream(stream);
	}
	sprite_mappings->toFile(directory + "map.asm", SpriteMappings::Format::ASSEMBLY);
	tile_manager.getTiles().toFile(directory + "tiles.bin", Tiles::Format::BINARY);

	if (slot != 0)
	{
		const auto &slot_from_number = [this](const int slot) constexpr
		{
			switch (slot)
			{
				case 1: return ui->actionLoad_Backup_1;
				case 2: return ui->actionLoad_Backup_2;
				case 3: return ui->actionLoad_Backup_3;
				case 4: return ui->actionLoad_Backup_4;
				case 5: return ui->actionLoad_Backup_5;
				case 6: return ui->actionLoad_Backup_6;
				case 7: return ui->actionLoad_Backup_7;
				case 8: return ui->actionLoad_Backup_8;
				case 9: return ui->actionLoad_Backup_9;
			}

			throw std::out_of_range("Slot number was not between 1 and 9");
		};

		slot_from_number(slot)->setDisabled(false);
	}
}

void MainWindow::LoadState(const int slot)
{
	QSettings settings;
	const QString directory = GetSlotDirectory(slot);

	// Load palette.
	palette.modify([&directory](Palette &palette)
	{
		try
		{
			QFile file(directory + "palette.bin");
			if (file.open(QFile::OpenModeFlag::ReadOnly))
			{
				DataStream stream(&file);
				palette.fromDataStream(stream);
			}
		}
		catch (...)
		{
			palette.reset();
		}
	});
	// Load mappings.
	try
	{
		sprite_mappings.modify([&directory](SpriteMappings &sprite_mappings)
		{
			sprite_mappings = SpriteMappings(directory + "map.asm", SpriteMappings::Format::ASSEMBLY);
		});
		const QString prefix = GetSlotPrefix(slot);
		const auto sprite = settings.value(prefix + "SelectedSpriteIndex", 0).toInt();
		sprite_viewer.setSelectedSprite(sprite == -1 ? std::optional<int>() : sprite);
		const auto piece = settings.value(prefix + "SelectedPieceIndex", 0).toInt();
		sprite_viewer.setSelectedPiece(piece == -1 ? std::optional<int>() : piece);
	}
	catch (...) {}
	// Load tiles.
	try
	{
		tile_manager.loadTilesFromFile(directory + "tiles.bin", Tiles::Format::BINARY);
	}
	catch (...) {}
}
