#include "main-window.h"

#include <QApplication>

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	// Qt6 does this by default
	QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
	QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

	QApplication a(argc, argv);
	MainWindow w;
	w.show();
	return a.exec();
}
