#include "frmSetting.h"
#include "E-N TabConverter/converter.h"
#include <QSettings>
#include <QMessageBox>
#include <QFileDialog>

frmSetting::frmSetting(QWidget* parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	QSettings s("settings.ini", QSettings::IniFormat);
	s.setIniCodec("UTF8");

	s.beginGroup("Train");
	ui.txtDataPth->setText(s.value("DataPath", "C:\\Users\\Administrator\\Desktop\\E-N TabConverter\\tData.csv").toString());
	ui.txtSmpPath->setText(s.value("SamplePath", "C:\\Users\\Administrator\\Desktop\\E-N TabConverter\\sample_classified\\").toString());

	s.beginGroup("Recognize");
	ui.txtSavePath->setText(s.value("SaveTo", "C:\\Users\\Administrator\\Desktop\\oh").toString());
	ui.txtCfgPath->setText(s.value("ConfigPath", "C:\\Users\\Administrator\\Desktop\\E-N TabConverter\\global.xml").toString());
	ui.txtOutputDir->setText(s.value("OutputDir").toString());
	changed = false;
}

QString frmSetting::selectDir(QString title, QString dir)
{
	return QFileDialog::getExistingDirectory(this, title, dir);
}

/*
	show a FileDialog to select one file. 
	@param title	title of the dialog. 
	@param filter	QString, like "Image(.bmp;*.jpg;*.JPG;*.jpeg;*.png;)"
	@retval	empty if the dialog is closed. else the path selected. 
*/
QString frmSetting::selectFile(QString title, QString dir, QString filter){
	return QFileDialog::getOpenFileName(this, title, dir, filter);
}

void frmSetting::onTrain()
{
	if (QMessageBox::No == QMessageBox::information(this, QString::fromLocal8Bit("重新训练"), 
		QString::fromLocal8Bit("您确实要重新训练吗? 这将清除所有已有的数据. "),
		QMessageBox::Yes | QMessageBox::No)) {
		return;
	}
	Converter::Train();
	QMessageBox::information(this, QString::fromLocal8Bit("训练已完成"), 
		QString::fromLocal8Bit("训练已完成, 数据位于") + ui.txtDataPth->text());
}

void frmSetting::onSave()
{
	QSettings s("settings.ini", QSettings::IniFormat);
	s.setValue("Train/DataPath", ui.txtDataPth->text());
	s.setValue("Train/SamplePath", ui.txtSmpPath->text());
	s.setValue("Recognize/SaveTo", ui.txtSavePath->text());
	s.setValue("Recognize/ConfigPath", ui.txtCfgPath->text());
	s.setValue("Recognize/OutputDir", ui.txtOutputDir->text());
	close();
}

void frmSetting::closeEvent(QCloseEvent* event)
{
	if (changed && QMessageBox::Yes == QMessageBox::question(this, "Unsaved Changes", "Save your changes? ")) {
		onSave();
	}
}

void frmSetting::selectSample()
{
	if (auto s = selectDir("select a directory"); !s.isEmpty())
		ui.txtSmpPath->setText(s);
}

void frmSetting::selectSave()
{
	if (auto s = selectDir("save the images in which directory? "); !s.isEmpty())
		ui.txtSavePath->setText(s);
}

void frmSetting::selectOutput()
{
	if (auto s = selectDir(selectDir("save output xml in which directory? ")); !s.isEmpty())
		ui.txtOutputDir->setText(s);
}

void frmSetting::selectData()
{
	if (auto s = selectDir(selectFile("select your csv file. ")); !s.isEmpty())
		ui.txtDataPth->setText(s);
}

void frmSetting::selectConfig()
{
	if (auto s = selectDir(selectFile("select your config XML. ")); !s.isEmpty())
		ui.txtCfgPath->setText(s);
}

