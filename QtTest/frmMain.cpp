#include "frmMain.h"
#include "converter.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QMimeData>
#include <QSettings>

frmMain::frmMain(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	ui.statusBar->showMessage("press 'Scan' to start. ");
	QSettings s("settings.ini", QSettings::IniFormat);
	s.setIniCodec("UTF8");
	outputDir = s.value("Recognize/OutputDir", ".").toString();
}

frmMain::~frmMain()
{
	delete setting;
}

void frmMain::dragEnterEvent(QDragEnterEvent* event)
{
	if (event->mimeData()->hasFormat("text/uri-list")) {
		event->acceptProposedAction();
	}
}

void frmMain::dropEvent(QDropEvent* e)
{
	QList<QUrl> urls = e->mimeData()->urls();
	for (QUrl tmp : urls) {
		if (auto s = tmp.toLocalFile(); s.isEmpty()) continue;
		else ui.listWidget->acceptIcon(s);
	}
}

void frmMain::onScan() {
	if (ui.listWidget->count() == 0) return;
	vector<string> list;
	for (int i = 0; i < ui.listWidget->count(); i++) {
		list.emplace_back(ui.listWidget->item(i)->text().toLocal8Bit());
	}
	Converter converter(list);
	converter.setCut(ui.ckbCut->isChecked());
	converter.setOutputDir(string(outputDir.toLocal8Bit()));
	converter.setSelectSaveStrategy([this]() -> string {
		return QFileDialog::getSaveFileName(this, "select where to save", 
			outputDir, "XML files (*.xml);;All files (*)").toLocal8Bit(); 
	});
	try {
		converter.scan(
			[this](string x) {ui.statusBar->showMessage(QString::fromLocal8Bit(x.data())); },
			[this](int x) {ui.progressBar->setValue(x); }
		);
	}
	catch (std::runtime_error ex) {
		ui.statusBar->showMessage(ex.what());
	}
	ui.listWidget->clear();
}

void frmMain::onSetting() {
	if(!setting) setting = new frmSetting(this);
	setting->show();
}

void frmMain::showAbout() {
	QMessageBox::about(this, "About", "Developed by JamzumSum. \n"
		"Credit: OpenCV");
}