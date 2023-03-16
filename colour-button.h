#ifndef COLOUR_BUTTON_H
#define COLOUR_BUTTON_H

#include <QAbstractButton>
#include <QColor>
#include <QObject>

class ColourButton : public QAbstractButton
{
	Q_OBJECT;

public:
	ColourButton();

	const QColor& colour() const {return this->m_colour;}
	void setColour(const QColor &colour) {this->m_colour = colour;}

protected:
	void paintEvent(QPaintEvent *event) override;

private:
	QColor m_colour;
};

#endif // COLOUR_BUTTON_H
