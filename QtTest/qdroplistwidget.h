#pragma once
#include <QListWidget>

class QDropListWidget: public QListWidget{

	Q_OBJECT

public:
	QDropListWidget(QWidget*);

	void acceptIcon(QString path);

public slots:
	void showItem(QListWidgetItem* item);
};