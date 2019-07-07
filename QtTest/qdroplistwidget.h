#pragma once
#include <QListWidget>
#include <QMenu>
#include <QDropEvent>

class QDropListWidget: public QListWidget{

	Q_OBJECT
private:
	QMenu pop;
	QList<QAction*> actions;
	QMenu itemPop;
	QList<QAction*> itemActions;

public:
	QDropListWidget(QWidget*);
	void acceptIcon(QString path);
	static QString outputDir;
	void showContextMenu(QListWidgetItem* item);
	void showContextMenu();
	~QDropListWidget();

protected:
	void mousePressEvent(QMouseEvent* event);

public slots:
	void onAddAction();
	void showItem(QListWidgetItem* item);
	void moveUpward();
	void moveDownward();
	void removeSelected();
};

class QPathIconItem : public QListWidgetItem {
private:
	static QRegExp rx1;
	QString path;

public:
	QPathIconItem(const QIcon& icon, const QString& path, QListWidget* parent = 0): path(path) {
		setText(path);
		setIcon(icon);
	}

	void setText(const QString& path) {
		this->path = path;
		int pos = rx1.indexIn(path);
		assert(pos != -1);
		QListWidgetItem::setText(rx1.cap(0));
	}

	QString text() const {
		return path;
	}
};