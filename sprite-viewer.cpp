#include "sprite-viewer.h"

#include <QPainter>
#include <QPainterPath>

#include "utilities.h"

SpriteViewer::SpriteViewer(const TileManager &tile_manager, const SpriteMappingsManager &sprite_mappings_manager)
	: sprite_mappings(sprite_mappings_manager.sprite_mappings())
    , tile_manager(tile_manager)
{
	setAutoFillBackground(true);

	connect(&tile_manager, &TileManager::regenerated, this, qOverload<>(&SpriteViewer::update));

	connect(&sprite_mappings_manager, &SpriteMappingsManager::mappingsModified, this,
		[this]()
		{
			if (sprite_mappings.frames.size() == 0)
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

	// Draw outline around selected sprite.
	const SpriteFrame &selected_sprite = frames[m_selected_sprite_index];
	const QColor background_colour = palette().color(QPalette::Window);

	QBrush brush;
	brush.setColor(QColor(background_colour.red() ^ 0xFF, background_colour.green() ^ 0xFF, background_colour.blue() / 2));
	brush.setStyle(Qt::BrushStyle::SolidPattern);

	painter.setPen(Qt::NoPen);
	painter.setBrush(brush);

	auto process_frame_or_piece = [this](std::function<void(const SpritePiece &piece)> callback)
	{
		auto &frame = sprite_mappings.frames[selected_sprite_index()];

		if (m_selected_piece_index != -1)
		{
			callback(frame.pieces[m_selected_piece_index]);
		}
		else
		{
			for (auto &piece : frame.pieces)
				callback(piece);
		}
	};

	QPainterPath combined_outline_path;

	process_frame_or_piece(
		[&combined_outline_path](const SpritePiece &piece)
		{
			// This algorithm is akward and weird, but it's all I can think of to recreate SonMapEd's outline effect.
			for (int y = 0; y < piece.height; ++y)
			{
				for (int x = 0; x < piece.width; ++x)
				{
					// Scale everything by 2 because the coordinate system here is twice the normal resolution.
					const QRect rect((piece.x + x * Tile::WIDTH) * 2, (piece.y + y * Tile::HEIGHT) * 2, Tile::WIDTH * 2, Tile::HEIGHT * 2);

					QPainterPath inner_path;
					inner_path.addRect(rect.marginsAdded(QMargins(1, 1, 1, 1)));

					QPainterPath outer_path;
					outer_path.addRect(rect.marginsRemoved(QMargins(1, 1, 1, 1)));

					const QPainterPath tile_outline_path(inner_path - outer_path);

					// Union minus intersection.
					combined_outline_path = (combined_outline_path + tile_outline_path) - (combined_outline_path & tile_outline_path);
				}
			}
		}
	);

	// Scale to double-resolution so that the outline isn't twice as thick as it should be.
	transform.scale(0.5, 0.5);
	painter.setTransform(transform);

	painter.drawPath(combined_outline_path);

	// Scale back to normal resolution.
	transform.scale(2.0, 2.0);
	painter.setTransform(transform);

	// Draw selected sprite.
	selected_sprite.draw(painter, tile_manager, starting_palette_line);

	int x_offset;

	// Draw sprites to the left of the selected sprite.
	x_offset = 0;

	for (int i = m_selected_sprite_index; i-- > 0; )
	{
		x_offset += qMin(-16, frames[i + 1].left());
		x_offset -= frames[i].right();
		frames[i].draw(painter, tile_manager, starting_palette_line, x_offset, 0);
	}

	// Draw sprites to the right of the selected sprite.
	x_offset = 0;

	for (int i = m_selected_sprite_index + 1; i < frames.size(); ++i)
	{
		x_offset += qMax(16, frames[i - 1].right());
		x_offset -= frames[i].left();
		frames[i].draw(painter, tile_manager, starting_palette_line, x_offset, 0);
	}

	// TODO: Remove.
	//painter.setPen(Qt::blue);
	//painter.setFont(QFont("Arial", 30));
	//painter.drawText(rect(), Qt::AlignCenter, QString(std::to_string(0).c_str()));
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
