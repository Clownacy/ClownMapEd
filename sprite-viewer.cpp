#include "sprite-viewer.h"

#include <QPainter>

SpriteViewer::SpriteViewer(const TilePixmaps &tile_pixmaps, const SpriteMappings &sprite_mappings)
	: QWidget()
	, selected_sprite_index(0)
	, pixmap("oozlostbackground2.png")
    , sprite_mappings(sprite_mappings)
    , tile_pixmaps(tile_pixmaps)
{
	setAutoFillBackground(true);
}

void SpriteViewer::paintFrame(QPainter &painter, unsigned int frame_index, int x_offset, int y_offset)
{
	const SpriteMappings::Frame &frame = sprite_mappings.frames[frame_index];

	for (unsigned int current_piece = 0; current_piece < frame.total_pieces; ++current_piece)
	{
		const SpriteMappings::Piece &piece = frame.pieces[current_piece];

		const QTransform flip_transform = QTransform::fromScale(piece.x_flip ? -1 : 1, piece.y_flip ? -1 : 1);

		unsigned int tile_index = piece.tile_index;

		for (int tile_x = 0; tile_x < piece.width; ++tile_x)
		{
			const int tile_x_corrected = piece.x_flip ? piece.width - tile_x - 1 : tile_x;

			for (int tile_y = 0; tile_y < piece.height; ++tile_y)
			{
				const int tile_y_corrected = piece.y_flip ? piece.height - tile_y - 1 : tile_y;

				const QRect rect(
					x_offset + piece.x + tile_x_corrected * 8,
					y_offset + piece.y + tile_y_corrected * 8,
					8,
					8
				);

				painter.drawPixmap(rect, tile_pixmaps[tile_index++].transformed(flip_transform), QRectF(0, 0, 8, 8));
			}
		}
	}
}

void SpriteViewer::paintEvent(QPaintEvent* const event)
{
	QWidget::paintEvent(event);

	QPainter painter(this);

	const qreal dpi_scale_x = qRound((qreal)logicalDpiX() / 96);
	const qreal dpi_scale_y = qRound((qreal)logicalDpiY() / 96);

	QTransform transform;
	transform.translate(width() / 2, height() / 2); // Centre origin.
	transform.scale(dpi_scale_x, dpi_scale_y); // Apply DPI scale.
	transform.scale(1.5, 1.5); // Apply general scale.
	painter.setTransform(transform);

	const SpriteMappings::Frame *frames = sprite_mappings.frames.data();

	if (sprite_mappings.frames.size() == 0)
		return;

	// Draw outline around selected sprite.
	const SpriteMappings::Frame &selected_sprite = frames[selected_sprite_index];
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
	paintFrame(painter, selected_sprite_index);

	int x_offset;

	// Draw sprites to the left of the selected sprite.
	x_offset = 0;

	for (unsigned int i = selected_sprite_index; i-- > 0; )
	{
		x_offset += qMin(-16, frames[i + 1].x1);
		x_offset -= frames[i].x2;
		paintFrame(painter, i, x_offset, 0);
	}

	// Draw sprites to the right of the selected sprite.
	x_offset = 0;

	for (unsigned int i = selected_sprite_index + 1; i < sprite_mappings.frames.size(); ++i)
	{
		x_offset += qMax(16, frames[i - 1].x2);
		x_offset -= frames[i].x1;
		paintFrame(painter, i, x_offset, 0);
	}

	// TODO: Remove.
	//painter.setPen(Qt::blue);
	//painter.setFont(QFont("Arial", 30));
	//painter.drawText(rect(), Qt::AlignCenter, QString(std::to_string(0).c_str()));
}

void SpriteViewer::selectNextSprite()
{
	if (selected_sprite_index != sprite_mappings.frames.size() - 1)
	{
		++selected_sprite_index;
		repaint();
	}
}

void SpriteViewer::selectPreviousSprite()
{
	if (selected_sprite_index != 0)
	{
		--selected_sprite_index;
		repaint();
	}
}

void SpriteViewer::setBackgroundColour(const QColor &colour)
{
	QPalette palette;
	palette.setColor(QPalette::Window, colour);
	setPalette(palette);
}
