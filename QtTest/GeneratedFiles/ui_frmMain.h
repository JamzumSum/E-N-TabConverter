/********************************************************************************
** Form generated from reading UI file 'frmMain.ui'
**
** Created by: Qt User Interface Compiler version 5.11.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FRMMAIN_H
#define UI_FRMMAIN_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_frmMainClass
{
public:
    QAction *actionHistory;
    QAction *actionExit;
    QWidget *centralWidget;
    QPushButton *btnScan;
    QSplitter *splitter_2;
    QCheckBox *checkBox_2;
    QCheckBox *checkBox;
    QPushButton *pushButton;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QMenu *menuAbout;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *frmMainClass)
    {
        if (frmMainClass->objectName().isEmpty())
            frmMainClass->setObjectName(QStringLiteral("frmMainClass"));
        frmMainClass->resize(840, 600);
        QFont font;
        font.setFamily(QStringLiteral("Microsoft YaHei UI"));
        font.setPointSize(10);
        frmMainClass->setFont(font);
        actionHistory = new QAction(frmMainClass);
        actionHistory->setObjectName(QStringLiteral("actionHistory"));
        actionExit = new QAction(frmMainClass);
        actionExit->setObjectName(QStringLiteral("actionExit"));
        centralWidget = new QWidget(frmMainClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        btnScan = new QPushButton(centralWidget);
        btnScan->setObjectName(QStringLiteral("btnScan"));
        btnScan->setGeometry(QRect(350, 250, 140, 50));
        splitter_2 = new QSplitter(centralWidget);
        splitter_2->setObjectName(QStringLiteral("splitter_2"));
        splitter_2->setGeometry(QRect(700, 460, 140, 80));
        splitter_2->setOrientation(Qt::Vertical);
        checkBox_2 = new QCheckBox(splitter_2);
        checkBox_2->setObjectName(QStringLiteral("checkBox_2"));
        splitter_2->addWidget(checkBox_2);
        checkBox = new QCheckBox(splitter_2);
        checkBox->setObjectName(QStringLiteral("checkBox"));
        splitter_2->addWidget(checkBox);
        pushButton = new QPushButton(centralWidget);
        pushButton->setObjectName(QStringLiteral("pushButton"));
        pushButton->setGeometry(QRect(8, 8, 48, 48));
        QIcon icon;
        icon.addFile(QStringLiteral("../icon/setting.ico"), QSize(), QIcon::Normal, QIcon::Off);
        pushButton->setIcon(icon);
        pushButton->setIconSize(QSize(40, 40));
        frmMainClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(frmMainClass);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 840, 26));
        QFont font1;
        font1.setFamily(QStringLiteral("Microsoft YaHei UI"));
        font1.setPointSize(9);
        menuBar->setFont(font1);
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName(QStringLiteral("menuFile"));
        menuAbout = new QMenu(menuBar);
        menuAbout->setObjectName(QStringLiteral("menuAbout"));
        frmMainClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(frmMainClass);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        frmMainClass->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(frmMainClass);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        frmMainClass->setStatusBar(statusBar);

        menuBar->addAction(menuFile->menuAction());
        menuBar->addAction(menuAbout->menuAction());
        menuFile->addAction(actionHistory);
        menuFile->addSeparator();
        menuFile->addAction(actionExit);

        retranslateUi(frmMainClass);

        QMetaObject::connectSlotsByName(frmMainClass);
    } // setupUi

    void retranslateUi(QMainWindow *frmMainClass)
    {
        frmMainClass->setWindowTitle(QApplication::translate("frmMainClass", "E-N Tab Converter", nullptr));
        actionHistory->setText(QApplication::translate("frmMainClass", "History", nullptr));
        actionExit->setText(QApplication::translate("frmMainClass", "Exit", nullptr));
        btnScan->setText(QApplication::translate("frmMainClass", "Scan", nullptr));
        checkBox_2->setText(QApplication::translate("frmMainClass", "Cut", nullptr));
        checkBox->setText(QApplication::translate("frmMainClass", "Save Image", nullptr));
        pushButton->setText(QString());
        menuFile->setTitle(QApplication::translate("frmMainClass", "File", nullptr));
        menuAbout->setTitle(QApplication::translate("frmMainClass", "About", nullptr));
    } // retranslateUi

};

namespace Ui {
    class frmMainClass: public Ui_frmMainClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FRMMAIN_H
