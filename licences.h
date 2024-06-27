#ifndef LICENCES_H
#define LICENCES_H

#include <QDialog>

namespace Ui {
class Licences;
}

class Licence : public QDialog
{
	Q_OBJECT

public:
	explicit Licence(QWidget *parent = nullptr);
	~Licence();

private:
	Ui::Licences *ui;
};

#endif // LICENCE_H
