#include "licences.h"
#include "ui_licence.h"

Licence::Licence(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::Licence)
{
	ui->setupUi(this);

	ui->tabWidget->setCurrentIndex(0);
	setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
}

Licence::~Licence()
{
	delete ui;
}
