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
	ui.statusBar->showMessage(QString::fromLocal8Bit("拖动曲谱到列表, 点击`Scan`"));
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
	if (ui.listWidget->count() == 0) {
		ui.statusBar->showMessage(QString::fromLocal8Bit("列表是空的啊(＃°Д°)"));
		return;
	}
	vector<string> list;
	for (int i = 0; i < ui.listWidget->count(); i++) {
		list.emplace_back(reinterpret_cast<QPathIconItem*>(ui.listWidget->item(i))->text().toLocal8Bit());
	}

	QSettings qs("settings.ini", QSettings::IniFormat);
	Converter converter(list);
	converter.Cut = ui.ckbCut->isChecked();
	converter.SavePic = ui.ckbSave->isChecked();
	converter.OutputDir = QDropListWidget::outputDir.toLocal8Bit().toStdString();
	converter.SelectSaveStrategy = [this]() -> string {
		return QFileDialog::getSaveFileName(this, QString::fromLocal8Bit("选择存储位置"), 
			QDropListWidget::outputDir, "XML files (*.xml);;All files (*)").toLocal8Bit().toStdString();
	};
	converter.CfgPath = qs.value("Recognize/ConfigPath", "./RecData/global.xml").toByteArray().toStdString();
	converter.CSVPath = qs.value("Train/DataPath", "./TrainData/tData.csv").toByteArray().toStdString();
	converter.PicSavePath = qs.value("Recognize/SaveTo", "./sample/unclassified").toByteArray().toStdString();
	converter.SamplePath = qs.value("Train/SamplePath", "./sample/classified").toByteArray().toStdString();

	progress(0);
	try {
		string outputfile = converter.scan(
			[this](string x) {ui.statusBar->showMessage(QString::fromLocal8Bit(x.data())); },
			progress
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
	constexpr auto ABOUT =
		"Version %s"
		"TinyXML version %s\n"
		"OpenCV version %s\n"
		"Developed by JamzumSum. \n";
	constexpr int bufCount = 1024;
	char buf[bufCount];
	snprintf(buf, bufCount, ABOUT, Converter::Version(), Converter::TinyXMLVersion(), Converter::cvVersion());
	QMessageBox::about(this, "About", buf);
}