#pragma once

#include <QDialog>
#include <QSettings>
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
	QString selectDir(QString title, QString dir = QString());
	QString selectFile(QString title, QString dir = QString(), QString filter = QString());
	QSettings qs;

public slots:
	void onTrain();
	void onSave();
	void textChange(QString) { changed = true;}
	void closeEvent(QCloseEvent* event);
	void selectSample();
	void selectSave();
	void selectOutput();
	void selectData();
	void selectConfig();
};
