#include "qdroplistwidget.h"
#include <QAbstractItemView>
#include <QListView>
#include <QMessageBox>
#include <QListWidgetItem>
#include <QDesktopServices>
#include <QUrl>

QDropListWidget::QDropListWidget(QWidget* w) :QListWidget(w) {
	setViewMode(QListView::IconMode);
	setIconSize(QSize(240, 420));
	setSpacing(16);
	setResizeMode(QListWidget::Adjust);
	setMovement(QListWidget::Static);
}

void QDropListWidget::acceptIcon(QString path)
{
	//定义QListWidgetItem对象
	QListWidgetItem* imageItem = new QListWidgetItem;
	imageItem->setIcon(QIcon(path));
	imageItem->setText(path);
	//重新设置单元项图片的宽度和高度
	imageItem->setSizeHint(QSize(240, 420));
	//将单元项添加到QListWidget中
	addItem(imageItem);
}

void QDropListWidget::showItem(QListWidgetItem* item)
{
	QString path = tr("file:///") + item->text();
	bool is_open = QDesktopServices::openUrl(QUrl(path));
	assert(is_open);
}
