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
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>
#include "qdroplistwidget.h"

QT_BEGIN_NAMESPACE

class Ui_frmMainClass
{
public:
    QAction *actionHistory;
    QAction *actionExit;
    QAction *actionAbout;
    QAction *actionSetting;
    QWidget *centralWidget;
    QGridLayout *gridLayout;
    QDropListWidget *listWidget;
    QSplitter *splitter;
    QProgressBar *progressBar;
    QPushButton *btnScan;
    QSplitter *splitter_checks;
    QCheckBox *ckbCut;
    QCheckBox *ckbSave;
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
        frmMainClass->setAcceptDrops(true);
        actionHistory = new QAction(frmMainClass);
        actionHistory->setObjectName(QStringLiteral("actionHistory"));
        actionExit = new QAction(frmMainClass);
        actionExit->setObjectName(QStringLiteral("actionExit"));
        actionAbout = new QAction(frmMainClass);
        actionAbout->setObjectName(QStringLiteral("actionAbout"));
        actionSetting = new QAction(frmMainClass);
        actionSetting->setObjectName(QStringLiteral("actionSetting"));
        centralWidget = new QWidget(frmMainClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        gridLayout = new QGridLayout(centralWidget);
        gridLayout->setSpacing(8);
        gridLayout->setContentsMargins(8, 8, 8, 8);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        listWidget = new QDropListWidget(centralWidget);
        listWidget->setObjectName(QStringLiteral("listWidget"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(7);
        sizePolicy.setHeightForWidth(listWidget->sizePolicy().hasHeightForWidth());
        listWidget->setSizePolicy(sizePolicy);
        listWidget->setAcceptDrops(true);
        listWidget->setDragEnabled(true);
        listWidget->setDragDropMode(QAbstractItemView::DragDrop);
        listWidget->setDefaultDropAction(Qt::MoveAction);

        gridLayout->addWidget(listWidget, 0, 0, 1, 1);

        splitter = new QSplitter(centralWidget);
        splitter->setObjectName(QStringLiteral("splitter"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(1);
        sizePolicy1.setHeightForWidth(splitter->sizePolicy().hasHeightForWidth());
        splitter->setSizePolicy(sizePolicy1);
        splitter->setFrameShadow(QFrame::Plain);
        splitter->setOrientation(Qt::Horizontal);
        progressBar = new QProgressBar(splitter);
        progressBar->setObjectName(QStringLiteral("progressBar"));
        QSizePolicy sizePolicy2(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sizePolicy2.setHorizontalStretch(6);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(progressBar->sizePolicy().hasHeightForWidth());
        progressBar->setSizePolicy(sizePolicy2);
        progressBar->setMinimumSize(QSize(512, 32));
        progressBar->setMaximumSize(QSize(16777215, 32));
        progressBar->setValue(0);
        splitter->addWidget(progressBar);
        btnScan = new QPushButton(splitter);
        btnScan->setObjectName(QStringLiteral("btnScan"));
        QSizePolicy sizePolicy3(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy3.setHorizontalStretch(1);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(btnScan->sizePolicy().hasHeightForWidth());
        btnScan->setSizePolicy(sizePolicy3);
        btnScan->setMinimumSize(QSize(90, 48));
        btnScan->setMaximumSize(QSize(16777214, 48));
        splitter->addWidget(btnScan);
        splitter_checks = new QSplitter(splitter);
        splitter_checks->setObjectName(QStringLiteral("splitter_checks"));
        QSizePolicy sizePolicy4(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy4.setHorizontalStretch(1);
        sizePolicy4.setVerticalStretch(0);
        sizePolicy4.setHeightForWidth(splitter_checks->sizePolicy().hasHeightForWidth());
        splitter_checks->setSizePolicy(sizePolicy4);
        splitter_checks->setMinimumSize(QSize(120, 60));
        splitter_checks->setOrientation(Qt::Vertical);
        ckbCut = new QCheckBox(splitter_checks);
        ckbCut->setObjectName(QStringLiteral("ckbCut"));
        splitter_checks->addWidget(ckbCut);
        ckbSave = new QCheckBox(splitter_checks);
        ckbSave->setObjectName(QStringLiteral("ckbSave"));
        splitter_checks->addWidget(ckbSave);
        splitter->addWidget(splitter_checks);

        gridLayout->addWidget(splitter, 1, 0, 1, 1, Qt::AlignVCenter);

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
        QWidget::setTabOrder(btnScan, ckbCut);
        QWidget::setTabOrder(ckbCut, ckbSave);

        menuBar->addAction(menuFile->menuAction());
        menuBar->addAction(menuHelp->menuAction());
        menuFile->addAction(actionHistory);
        menuFile->addAction(actionSetting);
        menuFile->addSeparator();
        menuFile->addAction(actionExit);
        menuHelp->addAction(actionAbout);

        retranslateUi(frmMainClass);
        QObject::connect(actionAbout, SIGNAL(triggered()), frmMainClass, SLOT(showAbout()));
        QObject::connect(actionExit, SIGNAL(triggered()), frmMainClass, SLOT(close()));
        QObject::connect(actionSetting, SIGNAL(triggered()), frmMainClass, SLOT(onSetting()));
        QObject::connect(btnScan, SIGNAL(clicked()), frmMainClass, SLOT(onScan()));
        QObject::connect(listWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), listWidget, SLOT(showItem(QListWidgetItem*)));

        QMetaObject::connectSlotsByName(frmMainClass);
    } // setupUi

    void retranslateUi(QMainWindow *frmMainClass)
    {
        frmMainClass->setWindowTitle(QApplication::translate("frmMainClass", "E-N Tab Converter", nullptr));
        actionHistory->setText(QApplication::translate("frmMainClass", "History", nullptr));
        actionExit->setText(QApplication::translate("frmMainClass", "Exit", nullptr));
        actionAbout->setText(QApplication::translate("frmMainClass", "About...", nullptr));
        actionSetting->setText(QApplication::translate("frmMainClass", "Setting", nullptr));
        btnScan->setText(QApplication::translate("frmMainClass", "Scan", nullptr));
        ckbCut->setText(QApplication::translate("frmMainClass", "Cut", nullptr));
        ckbSave->setText(QApplication::translate("frmMainClass", "Save Image", nullptr));
        menuFile->setTitle(QApplication::translate("frmMainClass", "File", nullptr));
        menuHelp->setTitle(QApplication::translate("frmMainClass", "Help", nullptr));
    } // retranslateUi

};

namespace Ui {
    class frmMainClass: public Ui_frmMainClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FRMMAIN_H
