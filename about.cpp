#include "about.h"

#include <QApplication>

About::About(QWidget *parent) :
	QDialog(parent)
{
	ui.setupUi(this);

	setModal(true);
	setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

	layout()->setSizeConstraint(QLayout::SetFixedSize);

	// Replace the version placeholder with the real version.
	ui.label->setText(ui.label->text().replace("[VERSION]", QApplication::applicationVersion()));
}
