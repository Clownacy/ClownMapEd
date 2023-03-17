#include "main-window.h"

#include <QApplication>

int main(int argc, char *argv[])
{
	// Enable dark theme on Windows.
	// TODO: Check if the dark theme is enabled on Linux.
	qputenv("QT_QPA_PLATFORM", "windows:darkmode=2");

	QApplication a(argc, argv);
	MainWindow w;
	w.show();
	return a.exec();
}
