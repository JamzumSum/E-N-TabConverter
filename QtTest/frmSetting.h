#pragma once

#include <QDialog>
#include "ui_frmSetting.h"

class frmSetting : public QDialog
{
	Q_OBJECT

public:
	frmSetting(QWidget *parent = Q_NULLPTR);
	//~frmSetting();

private:
	Ui::frmSetting ui;
	bool changed;

public slots:
	void onTrain();
	void onSave();
	void textChange(QString) { changed = true;}
	void closeEvent(QCloseEvent* event);
};
