#include "main-window.h"

#include <QApplication>

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	// Qt6 does this by default.
	QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
	QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif
#endif
	// Set some properties which will be useful for saving settings with QSettings.
	// The version is used for displaying the version in the About menu.
	QApplication::setApplicationName("ClownMapEd");
	QApplication::setApplicationVersion("1.2.0.2");
	QApplication::setOrganizationDomain("clownacy.com");
	QApplication::setOrganizationName("clownacy");

	QApplication a(argc, argv);
	MainWindow w;
	w.show();
	return a.exec();
}
