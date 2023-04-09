#include "sprite-viewer.h"

#include <QPainter>
#include <QPainterPath>

#include "utilities.h"

SpriteViewer::SpriteViewer(const TileManager &tile_manager, const SignalWrapper<SpriteMappings> &sprite_mappings)
	: sprite_mappings(*sprite_mappings)
    , tile_manager(tile_manager)
{
	setAutoFillBackground(true);

	connect(&tile_manager, &TileManager::pixmapsChanged, this, qOverload<>(&SpriteViewer::update));

	connect(&sprite_mappings, &SignalWrapper<SpriteMappings>::modified, this,
		[this]()
		{
			if (this->sprite_mappings.frames.size() == 0)
				m_selected_sprite_index = -1;
			else
				m_selected_sprite_index = qBound(0, m_selected_sprite_index, this->sprite_mappings.frames.size() - 1);

			update();
		}
	);
}

void SpriteViewer::paintEvent(QPaintEvent* const event)
{
	QWidget::paintEvent(event);

	QPainter painter(this);

	const QSizeF dpi_scale = Utilities::GetDPIScale(this);

	QTransform transform;
	transform.translate(width() / 2, height() / 2); // Centre origin.
	transform.scale(dpi_scale.width(), dpi_scale.height()); // Apply DPI scale.
	transform.scale(1.5, 1.5); // Apply general scale.
	painter.setTransform(transform);

	const QVector<SpriteFrame> &frames = sprite_mappings.frames;

	if (frames.size() == 0)
		return;

	//////////////////////////////////////////
	// Draw outline around selected sprite. //
	//////////////////////////////////////////

	const SpriteFrame &selected_sprite = frames[m_selected_sprite_index];
	const QColor background_colour = palette().color(QPalette::Window);

	QBrush brush;
	brush.setColor(QColor(background_colour.red() ^ 0xFF, background_colour.green() ^ 0xFF, background_colour.blue() / 2));
	brush.setStyle(Qt::BrushStyle::SolidPattern);

	painter.setPen(Qt::NoPen);
	painter.setBrush(brush);

	// Helper to perform an operation using each selected sprite piece.
	const auto process_selected_pieces = [this](const std::function<void(const SpritePiece &piece)> &callback)
	{
		auto &frame = sprite_mappings.frames[selected_sprite_index()];

		if (m_selected_piece_index != -1)
			callback(frame.pieces[m_selected_piece_index]);
		else
			for (auto &piece : frame.pieces)
				callback(piece);
	};

	// Helper to perform an operation using each tile of each selected sprite piece.
	const auto process_tiles_in_selected_pieces = [&process_selected_pieces](const std::function<void(const QRect &rect)> &callback)
	{
		process_selected_pieces(
			[&callback](const SpritePiece &piece)
			{
				for (int y = 0; y < piece.height; ++y)
					for (int x = 0; x < piece.width; ++x)
						callback(QRect(piece.x + x * TileManager::TILE_WIDTH, piece.y + y * TileManager::TILE_HEIGHT, TileManager::TILE_WIDTH, TileManager::TILE_HEIGHT));
			}
		);
	};

	// This algorithm is awkward and weird, but it's all I can think of to recreate SonMapEd's outline effect.
	// We create the outline with a `QPainterPath`, which undergoes various area subtractions and unions.
	QPainterPath combined_outline_path;

	process_tiles_in_selected_pieces(
		[&combined_outline_path](const QRect &rect)
		{
			// The general idea is to draw a 2-pixel border around each tile.
			// One pixel resides inside the bounds of the tile, and the other resides outside it.
			// This border is 2 pixels thick so that it erases both the inner and outer borders of neighbouring tiles.
			// To create this border, we create a rectangle that's slightly smaller than the tile,
			// and subtract from it a rectangle that is slightly larger than the tile.
			QPainterPath inner_path;
			inner_path.addRect(rect.marginsAdded(QMargins(1, 1, 1, 1)));

			QPainterPath outer_path;
			outer_path.addRect(rect.marginsRemoved(QMargins(1, 1, 1, 1)));

			const QPainterPath tile_outline_path(inner_path - outer_path);

			// Now that we have our tile border, we combine it with the others.
			// To erase the border of neighbouring tiles, we also subtract the intersection.
			combined_outline_path = (combined_outline_path | tile_outline_path) - (combined_outline_path & tile_outline_path);
		}
	);

	// At this point, we have our desired sprite outline, however
	// it is 2 pixels thick instead of 1, so we must correct this.
	process_selected_pieces(
		[&combined_outline_path](const SpritePiece &piece)
		{
			// To reduce the thickness of the outline, we create a series of piece borders,
			// which are only 1 pixel thick and reside exclusively outside of the piece's bounds.
			const QRect rect = piece.rect();

			QPainterPath inner_path;
			inner_path.addRect(rect.marginsAdded(QMargins(1, 1, 1, 1)));

			QPainterPath outer_path;
			outer_path.addRect(rect);

			const QPainterPath tile_outline_path(inner_path - outer_path);

			// We then subtract the intersection of these borders with the sprite outline.
			// This erases the outer part of the outline, leaving it only a single pixel thick.
			combined_outline_path -= combined_outline_path & tile_outline_path;
		}
	);

	// The sprite outline is finally complete, so we can now draw it.
	painter.drawPath(combined_outline_path);

	///////////////////
	// Draw sprites. //
	///////////////////

	// Draw selected sprite.
	if (m_selected_piece_index == -1)
		selected_sprite.draw(painter, tile_manager, TileManager::PixmapType::NO_BACKGROUND, 0, TileManager::PixmapType::NO_BACKGROUND, starting_palette_line);
	else
		selected_sprite.draw(painter, tile_manager, TileManager::PixmapType::TRANSPARENT, m_selected_piece_index, TileManager::PixmapType::NO_BACKGROUND, starting_palette_line);

	int x_offset;

	// Draw sprites to the left of the selected sprite.
	x_offset = 0;

	for (int i = m_selected_sprite_index; i-- > 0; )
	{
		x_offset += qMin(-16, frames[i + 1].left());
		x_offset -= frames[i].right();
		frames[i].draw(painter, tile_manager, TileManager::PixmapType::TRANSPARENT, 0, TileManager::PixmapType::TRANSPARENT, starting_palette_line, x_offset, 0);
	}

	// Draw sprites to the right of the selected sprite.
	x_offset = 0;

	for (int i = m_selected_sprite_index + 1; i < frames.size(); ++i)
	{
		x_offset += qMax(16, frames[i - 1].right());
		x_offset -= frames[i].left();
		frames[i].draw(painter, tile_manager, TileManager::PixmapType::TRANSPARENT, 0, TileManager::PixmapType::TRANSPARENT, starting_palette_line, x_offset, 0);
	}
}

void SpriteViewer::setSelectedSprite(const int sprite_index)
{
	m_selected_sprite_index = qBound(0, sprite_index, sprite_mappings.frames.size() - 1);
	m_selected_piece_index = -1;
	update();

	emit selectedSpriteChanged();
}

void SpriteViewer::setSelectedPiece(const int piece_index)
{
	m_selected_piece_index = ((piece_index + 1) % (sprite_mappings.frames[m_selected_sprite_index].pieces.size() + 1)) - 1;
	if (m_selected_piece_index < -1)
		m_selected_piece_index += sprite_mappings.frames[m_selected_sprite_index].pieces.size() + 1;
	update();

	emit selectedSpriteChanged();
}

void SpriteViewer::setBackgroundColour(const QColor &colour)
{
	QPalette palette;
	palette.setColor(QPalette::Window, colour);
	setPalette(palette);
}
