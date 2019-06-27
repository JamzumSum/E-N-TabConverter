#include "frmSetting.h"
#include "E-N TabConverter/converter.h"
#include <QSettings>
#include <QMessageBox>

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

void frmSetting::onTrain()
{
	Converter::Train();
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

