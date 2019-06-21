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
#include <QtWidgets/QProgressBar>
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
    QAction *actionAbout;
    QWidget *centralWidget;
    QPushButton *btnScan;
    QSplitter *splitter_checks;
    QCheckBox *ckbCut;
    QCheckBox *ckbSave;
    QPushButton *btnSetting;
    QProgressBar *progressBar;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QMenu *menuHelp;
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
        actionAbout = new QAction(frmMainClass);
        actionAbout->setObjectName(QStringLiteral("actionAbout"));
        centralWidget = new QWidget(frmMainClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        btnScan = new QPushButton(centralWidget);
        btnScan->setObjectName(QStringLiteral("btnScan"));
        btnScan->setGeometry(QRect(350, 250, 140, 50));
        splitter_checks = new QSplitter(centralWidget);
        splitter_checks->setObjectName(QStringLiteral("splitter_checks"));
        splitter_checks->setGeometry(QRect(696, 420, 140, 80));
        splitter_checks->setOrientation(Qt::Vertical);
        ckbCut = new QCheckBox(splitter_checks);
        ckbCut->setObjectName(QStringLiteral("ckbCut"));
        splitter_checks->addWidget(ckbCut);
        ckbSave = new QCheckBox(splitter_checks);
        ckbSave->setObjectName(QStringLiteral("ckbSave"));
        splitter_checks->addWidget(ckbSave);
        btnSetting = new QPushButton(centralWidget);
        btnSetting->setObjectName(QStringLiteral("btnSetting"));
        btnSetting->setGeometry(QRect(8, 8, 48, 48));
        QIcon icon;
        icon.addFile(QStringLiteral("../icon/setting.ico"), QSize(), QIcon::Normal, QIcon::Off);
        btnSetting->setIcon(icon);
        btnSetting->setIconSize(QSize(40, 40));
        progressBar = new QProgressBar(centralWidget);
        progressBar->setObjectName(QStringLiteral("progressBar"));
        progressBar->setGeometry(QRect(8, 504, 824, 24));
        progressBar->setValue(0);
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
        menuHelp = new QMenu(menuBar);
        menuHelp->setObjectName(QStringLiteral("menuHelp"));
        frmMainClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(frmMainClass);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        frmMainClass->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(frmMainClass);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        frmMainClass->setStatusBar(statusBar);
        QWidget::setTabOrder(btnScan, btnSetting);
        QWidget::setTabOrder(btnSetting, ckbCut);
        QWidget::setTabOrder(ckbCut, ckbSave);

        menuBar->addAction(menuFile->menuAction());
        menuBar->addAction(menuHelp->menuAction());
        menuFile->addAction(actionHistory);
        menuFile->addSeparator();
        menuFile->addAction(actionExit);
        menuHelp->addAction(actionAbout);

        retranslateUi(frmMainClass);
        QObject::connect(btnScan, SIGNAL(clicked()), frmMainClass, SLOT(onScan()));
        QObject::connect(btnSetting, SIGNAL(clicked()), frmMainClass, SLOT(onSetting()));
        QObject::connect(actionAbout, SIGNAL(triggered()), frmMainClass, SLOT(showAbout()));
        QObject::connect(actionExit, SIGNAL(triggered()), frmMainClass, SLOT(close()));

        QMetaObject::connectSlotsByName(frmMainClass);
    } // setupUi

    void retranslateUi(QMainWindow *frmMainClass)
    {
        frmMainClass->setWindowTitle(QApplication::translate("frmMainClass", "E-N Tab Converter", nullptr));
        actionHistory->setText(QApplication::translate("frmMainClass", "History", nullptr));
        actionExit->setText(QApplication::translate("frmMainClass", "Exit", nullptr));
        actionAbout->setText(QApplication::translate("frmMainClass", "About...", nullptr));
        btnScan->setText(QApplication::translate("frmMainClass", "Scan", nullptr));
        ckbCut->setText(QApplication::translate("frmMainClass", "Cut", nullptr));
        ckbSave->setText(QApplication::translate("frmMainClass", "Save Image", nullptr));
        btnSetting->setText(QString());
        menuFile->setTitle(QApplication::translate("frmMainClass", "File", nullptr));
        menuHelp->setTitle(QApplication::translate("frmMainClass", "Help", nullptr));
    } // retranslateUi

};

namespace Ui {
    class frmMainClass: public Ui_frmMainClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FRMMAIN_H
