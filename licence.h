#ifndef LICENCE_H
#define LICENCE_H

#include <QDialog>

namespace Ui {
class Licence;
}

class Licence : public QDialog
{
	Q_OBJECT

public:
	explicit Licence(QWidget *parent = nullptr);
	~Licence();

private:
	Ui::Licence *ui;
};

#endif // LICENCE_H
