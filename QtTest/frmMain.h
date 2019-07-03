#pragma once

#include <QtWidgets/QMainWindow>
#include <QDragEnterEvent>
#include <QDropEvent>
#include "frmSetting.h"
#include "ui_frmMain.h"

class frmMain : public QMainWindow
{
	Q_OBJECT

public:
	frmMain(QWidget *parent = Q_NULLPTR);
	~frmMain();
private:
	frmSetting* setting = nullptr;
	Ui::frmMainClass ui;
	QString outputDir;
protected:
	void dragEnterEvent(QDragEnterEvent* event);
	void dropEvent(QDropEvent* event);
	//void dragMoveEvent(QDragMoveEvent* event);

public slots:

	void onScan();
	void onSetting();
	void showAbout();
};
