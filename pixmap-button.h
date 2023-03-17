#ifndef PIXMAP_BUTTON_H
#define PIXMAP_BUTTON_H

#include <QAbstractButton>
#include <QObject>
#include <QPixmap>

class PixmapButton : public QAbstractButton
{
	Q_OBJECT;

public:
	using QAbstractButton::QAbstractButton;

	const QPixmap& pixmap() const { return m_pixmap; }
	void setPixmap(const QPixmap &pixmap) { m_pixmap = pixmap; }

protected:
	void paintEvent(QPaintEvent *event) override;

private:
	QPixmap m_pixmap;
};

#endif // PIXMAP_BUTTON_H
