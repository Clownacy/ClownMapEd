#include "about.h"
#include "ui_about.h"

About::About(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::About)
{
	ui->setupUi(this);

	setModal(true);
	setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

	this->setFixedSize(this->size());

	ui->labelIcon->setPixmap(QPixmap(":/Icon/ClownMapEd.png"));
}

About::~About()
{
	delete ui;
}
