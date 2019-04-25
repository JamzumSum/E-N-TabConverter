#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_frmMain.h"

class frmMain : public QMainWindow
{
	Q_OBJECT

public:
	frmMain(QWidget *parent = Q_NULLPTR);

private:
	Ui::frmMainClass ui;
};
