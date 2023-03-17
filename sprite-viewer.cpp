#include "sprite-viewer.h"

#include <QPainter>

#include "utilities.h"

SpriteViewer::SpriteViewer(const TileManager &tile_manager, const SpriteMappings &sprite_mappings)
	: sprite_mappings(sprite_mappings)
    , tile_manager(tile_manager)
{
	setAutoFillBackground(true);
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

	const QVector<SpriteFrame> &frames = sprite_mappings.frames();

	if (frames.size() == 0)
		return;

	// Draw outline around selected sprite.
	const SpriteFrame &selected_sprite = frames[selected_sprite_index];
	const QColor background_colour = palette().color(QPalette::Window);
	const QRect outline_rect(selected_sprite.x1, selected_sprite.y1, selected_sprite.x2 - selected_sprite.x1, selected_sprite.y2 - selected_sprite.y1);

	// Draw a solid colour outline.
	// This appears to be the exact formula that SonMapEd uses for its selection box.
	painter.setPen(QColor(background_colour.red() ^ 0xFF, background_colour.green() ^ 0xFF, background_colour.blue() / 2));
	painter.drawRect(outline_rect);

	// Draw some gaps into the outline, to make it resemble the one in SonMapEd.
	QBrush brush;
	brush.setStyle(Qt::BrushStyle::DiagCrossPattern);
	brush.setColor(background_colour);
	QPen pen;
	pen.setBrush(brush);
	painter.setPen(pen);
	painter.drawRect(outline_rect);

	// Draw selected sprite.
	selected_sprite.draw(painter, tile_manager);

	int x_offset;

	// Draw sprites to the left of the selected sprite.
	x_offset = 0;

	for (int i = selected_sprite_index; i-- > 0; )
	{
		x_offset += qMin(-16, frames[i + 1].x1);
		x_offset -= frames[i].x2;
		frames[i].draw(painter, tile_manager, x_offset, 0);
	}

	// Draw sprites to the right of the selected sprite.
	x_offset = 0;

	for (int i = selected_sprite_index + 1; i < frames.size(); ++i)
	{
		x_offset += qMax(16, frames[i - 1].x2);
		x_offset -= frames[i].x1;
		frames[i].draw(painter, tile_manager, x_offset, 0);
	}

	// TODO: Remove.
	//painter.setPen(Qt::blue);
	//painter.setFont(QFont("Arial", 30));
	//painter.drawText(rect(), Qt::AlignCenter, QString(std::to_string(0).c_str()));
}

void SpriteViewer::selectNextSprite()
{
	if (selected_sprite_index != sprite_mappings.frames().size() - 1)
	{
		++selected_sprite_index;
		update();
	}
}

void SpriteViewer::selectPreviousSprite()
{
	if (selected_sprite_index != 0)
	{
		--selected_sprite_index;
		update();
	}
}

void SpriteViewer::setBackgroundColour(const QColor &colour)
{
	QPalette palette;
	palette.setColor(QPalette::Window, colour);
	setPalette(palette);
}
