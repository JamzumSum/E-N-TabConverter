#include "qdroplistwidget.h"
#include <QAbstractItemView>
#include <QListView>
#include <QMessageBox>
#include <QListWidgetItem>
#include <QDesktopServices>
#include <QMimeData>
#include <QUrl>
#include <QFileDialog>
#include <QSettings>

QRegExp QPathIconItem::rx1 = QRegExp("([^/]+)\\..+");
QString QDropListWidget::outputDir = 
QSettings("settings.ini", QSettings::IniFormat).value("Recognize/OutputDir", ".").toString();

QDropListWidget::QDropListWidget(QWidget* w) :QListWidget(w), pop(this), itemPop(this) {
	setViewMode(QListView::IconMode);
	setIconSize(QSize(240, 420));
	setSpacing(16);
	setResizeMode(QListWidget::Adjust);
	setMovement(QListWidget::Static);
	actions.append(new QAction("Add", this));
	actions.append(new QAction("Clear", this));
	pop.addActions(actions);
	
	connect(actions[0], SIGNAL(triggered()), this, SLOT(onAddAction()));
	connect(actions[1], SIGNAL(triggered()), this, SLOT(clear()));

	itemActions.append(new QAction("Remove", this));
	itemActions.append(new QAction("Upward", this));
	itemActions.append(new QAction("Downward", this));
	itemPop.addActions(itemActions);

	connect(itemActions[0], SIGNAL(triggered()), this, SLOT(removeSelected));
	connect(itemActions[1], SIGNAL(triggered()), this, SLOT(moveUpward()));
	connect(itemActions[2], SIGNAL(triggered()), this, SLOT(moveDownward()));

}

void QDropListWidget::acceptIcon(QString path)
{
	//定义QListWidgetItem对象
	QListWidgetItem* imageItem = new QPathIconItem(QIcon(path), path);
	//重新设置单元项图片的宽度和高度
	imageItem->setSizeHint(QSize(240, 420));
	//将单元项添加到QListWidget中
	addItem(imageItem);
}

void QDropListWidget::mousePressEvent(QMouseEvent* event) {
	auto item = itemAt(event->pos());
	if (!item) clearSelection();

	switch (event->button()) {
	case Qt::RightButton:
		if (item) showContextMenu(item);
		else showContextMenu();
		break;
	case Qt::LeftButton:
		if(item) item->setSelected(true);
		break;
	}
}

void QDropListWidget::showItem(QListWidgetItem* item)
{
	QString path = tr("file:///") + item->text();
	bool is_open = QDesktopServices::openUrl(QUrl(path));
	assert(is_open);
}

void QDropListWidget::showContextMenu(QListWidgetItem* p) {
	clearSelection();
	p->setSelected(true);
	itemPop.exec(cursor().pos());
}

void QDropListWidget::showContextMenu() {
	pop.exec(cursor().pos());
}

void QDropListWidget::onAddAction() {
	auto list = QFileDialog::getOpenFileNames(this, "select images", 
		QDropListWidget::outputDir, "Image files (*.bmp, *.jpg, *.png);;All files (*)");
	for (const auto& i : list) acceptIcon(i);
}

QDropListWidget::~QDropListWidget() {
	for (QAction* i : actions) delete i;
}

void QDropListWidget::moveUpward() {
	auto item = selectedItems()[0];
	int r = row(item);
	if(r > 0);
		insertItem(r - 1, takeItem(r));
}

void QDropListWidget::moveDownward()
{
	auto item = selectedItems()[0];
	int r = row(item);
	if(r < count())
		insertItem(r + 1, takeItem(r));
}

void QDropListWidget::removeSelected() {
	removeItemWidget(selectedItems()[0]);
}
