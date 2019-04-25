#include "frmMain.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	frmMain w;
	w.show();
	return a.exec();
}
