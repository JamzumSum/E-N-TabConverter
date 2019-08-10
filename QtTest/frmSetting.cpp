#include "frmSetting.h"
#include "E-N TabConverter/converter.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>

frmSetting::frmSetting(QWidget* parent)
	: QDialog(parent), qs("settings.ini", QSettings::IniFormat)
{
	ui.setupUi(this);
	QSettings s("settings.ini", QSettings::IniFormat);
	s.setIniCodec("UTF8");

	s.beginGroup("Train");
	ui.txtDataPth->setText(s.value("DataPath", "tData.csv").toString());
	ui.txtSmpPath->setText(s.value("SamplePath", "sample_classified").toString());

	s.beginGroup("Recognize");
	ui.txtSavePath->setText(s.value("SaveTo", "sample_unclassified").toString());
	ui.txtCfgPath->setText(s.value("ConfigPath", "global.xml").toString());
	ui.txtOutputDir->setText(s.value("OutputDir", QStandardPaths::DocumentsLocation).toString());
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
	Converter converter;
	converter.CSVPath = qs.value("Train/DataPath").toByteArray().toStdString();
	converter.Train();
	QMessageBox::information(this, QString::fromLocal8Bit("训练已完成"), 
		QString::fromLocal8Bit("训练已完成, 数据位于") + ui.txtDataPth->text());
}

void frmSetting::onSave()
{
	qs.setValue("Train/DataPath", ui.txtDataPth->text());
	qs.setValue("Train/SamplePath", ui.txtSmpPath->text());
	qs.setValue("Recognize/SaveTo", ui.txtSavePath->text());
	qs.setValue("Recognize/ConfigPath", ui.txtCfgPath->text());
	qs.setValue("Recognize/OutputDir", ui.txtOutputDir->text());
	close();
}

void frmSetting::closeEvent(QCloseEvent* event)
{
	if (changed && QMessageBox::Yes == QMessageBox::question(this, 
		QString::fromLocal8Bit("未保存的更改"), "保存您的配置? ")) 
	{
		onSave();
	}
}

void frmSetting::selectSample()
{
	if (auto s = selectDir(QString::fromLocal8Bit("选择目录")); !s.isEmpty())
		ui.txtSmpPath->setText(s);
}

void frmSetting::selectSave()
{
	if (auto s = selectDir(QString::fromLocal8Bit("要保存到哪一目录? ")); !s.isEmpty())
		ui.txtSavePath->setText(s);
}

void frmSetting::selectOutput()
{
	if (auto s = selectDir(selectDir(QString::fromLocal8Bit("要保存到那一目录? "))); !s.isEmpty())
		ui.txtOutputDir->setText(s);
}

void frmSetting::selectData()
{
	if (auto s = selectDir(selectFile(QString::fromLocal8Bit("选择一个CSV文件"))); !s.isEmpty())
		ui.txtDataPth->setText(s);
}

void frmSetting::selectConfig()
{
	if (auto s = selectDir(selectFile(QString::fromLocal8Bit("选择一个XML配置文件"))); !s.isEmpty())
		ui.txtCfgPath->setText(s);
}

