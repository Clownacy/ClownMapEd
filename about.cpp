#include "about.h"
#include "ui_about.h"

#include <QApplication>

About::About(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::About)
{
	ui->setupUi(this);

	setModal(true);
	setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

	this->setFixedSize(this->size());

	ui->labelIcon->setPixmap(QPixmap(":/Icon/assets/icon/icon-master.png"));

	// Replace the version placeholder with the real version.
	ui->label->setText(ui->label->text().replace("[VERSION]", QApplication::applicationVersion()));
}

About::~About()
{
	delete ui;
}
