#pragma once

#include <QtWidgets/QMainWindow>
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
public slots:

	void onScan();
	void onSetting();
	void showAbout();
};
