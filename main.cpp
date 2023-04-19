#include "main-window.h"

#include <QApplication>

int main(int argc, char *argv[])
{
	// TODO: I think Ewan told me that Qt6 does this by default, so ditch this code if/when we switch from Qt5 to that!
	QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
	QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

	QApplication a(argc, argv);
	MainWindow w;
	w.show();
	return a.exec();
}
