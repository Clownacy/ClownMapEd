#include "licences.h"
#include "ui_licences.h"

Licence::Licence(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::Licences)
{
	ui->setupUi(this);

	ui->tabWidget->setCurrentIndex(0);
	setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
}

Licence::~Licence()
{
	delete ui;
}
