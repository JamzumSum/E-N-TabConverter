#include "frmMain.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	/*QFile qss("MaterialDark.qss");
	qss.open(QFile::ReadOnly);
	a.setStyleSheet(qss.readAll());*/
	frmMain w;
	w.show();
	return a.exec();
}
