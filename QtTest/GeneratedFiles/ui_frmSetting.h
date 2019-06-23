/********************************************************************************
** Form generated from reading UI file 'frmSetting.ui'
**
** Created by: Qt User Interface Compiler version 5.11.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FRMSETTING_H
#define UI_FRMSETTING_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_frmSetting
{
public:
    QVBoxLayout *verticalLayout_2;
    QScrollArea *scrollArea;
    QWidget *scrollAreaWidgetContents;
    QGroupBox *groupCfgPath;
    QSplitter *splitter;
    QLineEdit *txtCfgPath;
    QPushButton *btnCfgSelect;
    QGroupBox *groupSmpPath;
    QSplitter *splitter_2;
    QLineEdit *txtSmpPath;
    QPushButton *btnSmpSelect;
    QGroupBox *groupStudyData;
    QSplitter *splitter_4;
    QLineEdit *txtDataPth;
    QPushButton *btnDataSelect;
    QGroupBox *groupSaveFolder;
    QSplitter *splitter_3;
    QLineEdit *txtSavePath;
    QPushButton *btnSaveSelect;
    QPushButton *btnSave;
    QPushButton *btnTrain;
    QGroupBox *groupOutputDir;
    QSplitter *splitter_6;
    QLineEdit *txtOutputDir;
    QPushButton *btnOutputDir;

    void setupUi(QDialog *frmSetting)
    {
        if (frmSetting->objectName().isEmpty())
            frmSetting->setObjectName(QStringLiteral("frmSetting"));
        frmSetting->resize(720, 459);
        verticalLayout_2 = new QVBoxLayout(frmSetting);
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setContentsMargins(11, 11, 11, 11);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        scrollArea = new QScrollArea(frmSetting);
        scrollArea->setObjectName(QStringLiteral("scrollArea"));
        scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        scrollArea->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
        scrollArea->setWidgetResizable(true);
        scrollAreaWidgetContents = new QWidget();
        scrollAreaWidgetContents->setObjectName(QStringLiteral("scrollAreaWidgetContents"));
        scrollAreaWidgetContents->setGeometry(QRect(0, 0, 720, 640));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(8);
        sizePolicy.setHeightForWidth(scrollAreaWidgetContents->sizePolicy().hasHeightForWidth());
        scrollAreaWidgetContents->setSizePolicy(sizePolicy);
        scrollAreaWidgetContents->setMinimumSize(QSize(720, 640));
        groupCfgPath = new QGroupBox(scrollAreaWidgetContents);
        groupCfgPath->setObjectName(QStringLiteral("groupCfgPath"));
        groupCfgPath->setGeometry(QRect(12, 12, 673, 95));
        splitter = new QSplitter(groupCfgPath);
        splitter->setObjectName(QStringLiteral("splitter"));
        splitter->setGeometry(QRect(8, 32, 640, 32));
        splitter->setOrientation(Qt::Horizontal);
        splitter->setHandleWidth(8);
        txtCfgPath = new QLineEdit(splitter);
        txtCfgPath->setObjectName(QStringLiteral("txtCfgPath"));
        splitter->addWidget(txtCfgPath);
        btnCfgSelect = new QPushButton(splitter);
        btnCfgSelect->setObjectName(QStringLiteral("btnCfgSelect"));
        splitter->addWidget(btnCfgSelect);
        groupSmpPath = new QGroupBox(scrollAreaWidgetContents);
        groupSmpPath->setObjectName(QStringLiteral("groupSmpPath"));
        groupSmpPath->setGeometry(QRect(12, 120, 673, 96));
        splitter_2 = new QSplitter(groupSmpPath);
        splitter_2->setObjectName(QStringLiteral("splitter_2"));
        splitter_2->setGeometry(QRect(8, 32, 640, 32));
        splitter_2->setOrientation(Qt::Horizontal);
        splitter_2->setHandleWidth(8);
        txtSmpPath = new QLineEdit(splitter_2);
        txtSmpPath->setObjectName(QStringLiteral("txtSmpPath"));
        splitter_2->addWidget(txtSmpPath);
        btnSmpSelect = new QPushButton(splitter_2);
        btnSmpSelect->setObjectName(QStringLiteral("btnSmpSelect"));
        splitter_2->addWidget(btnSmpSelect);
        groupStudyData = new QGroupBox(scrollAreaWidgetContents);
        groupStudyData->setObjectName(QStringLiteral("groupStudyData"));
        groupStudyData->setGeometry(QRect(12, 228, 673, 95));
        splitter_4 = new QSplitter(groupStudyData);
        splitter_4->setObjectName(QStringLiteral("splitter_4"));
        splitter_4->setGeometry(QRect(8, 32, 640, 32));
        splitter_4->setOrientation(Qt::Horizontal);
        splitter_4->setHandleWidth(8);
        txtDataPth = new QLineEdit(splitter_4);
        txtDataPth->setObjectName(QStringLiteral("txtDataPth"));
        splitter_4->addWidget(txtDataPth);
        btnDataSelect = new QPushButton(splitter_4);
        btnDataSelect->setObjectName(QStringLiteral("btnDataSelect"));
        splitter_4->addWidget(btnDataSelect);
        groupSaveFolder = new QGroupBox(scrollAreaWidgetContents);
        groupSaveFolder->setObjectName(QStringLiteral("groupSaveFolder"));
        groupSaveFolder->setGeometry(QRect(12, 336, 673, 95));
        splitter_3 = new QSplitter(groupSaveFolder);
        splitter_3->setObjectName(QStringLiteral("splitter_3"));
        splitter_3->setGeometry(QRect(8, 32, 640, 32));
        splitter_3->setOrientation(Qt::Horizontal);
        splitter_3->setHandleWidth(8);
        txtSavePath = new QLineEdit(splitter_3);
        txtSavePath->setObjectName(QStringLiteral("txtSavePath"));
        splitter_3->addWidget(txtSavePath);
        btnSaveSelect = new QPushButton(splitter_3);
        btnSaveSelect->setObjectName(QStringLiteral("btnSaveSelect"));
        splitter_3->addWidget(btnSaveSelect);
        btnSave = new QPushButton(scrollAreaWidgetContents);
        btnSave->setObjectName(QStringLiteral("btnSave"));
        btnSave->setGeometry(QRect(570, 550, 96, 36));
        btnTrain = new QPushButton(scrollAreaWidgetContents);
        btnTrain->setObjectName(QStringLiteral("btnTrain"));
        btnTrain->setGeometry(QRect(450, 550, 96, 36));
        QPalette palette;
        QBrush brush(QColor(255, 0, 0, 255));
        brush.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::ButtonText, brush);
        palette.setBrush(QPalette::Inactive, QPalette::ButtonText, brush);
        QBrush brush1(QColor(120, 120, 120, 255));
        brush1.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Disabled, QPalette::ButtonText, brush1);
        btnTrain->setPalette(palette);
        groupOutputDir = new QGroupBox(scrollAreaWidgetContents);
        groupOutputDir->setObjectName(QStringLiteral("groupOutputDir"));
        groupOutputDir->setGeometry(QRect(20, 450, 673, 95));
        splitter_6 = new QSplitter(groupOutputDir);
        splitter_6->setObjectName(QStringLiteral("splitter_6"));
        splitter_6->setGeometry(QRect(8, 32, 640, 32));
        splitter_6->setOrientation(Qt::Horizontal);
        splitter_6->setHandleWidth(8);
        txtOutputDir = new QLineEdit(splitter_6);
        txtOutputDir->setObjectName(QStringLiteral("txtOutputDir"));
        splitter_6->addWidget(txtOutputDir);
        btnOutputDir = new QPushButton(splitter_6);
        btnOutputDir->setObjectName(QStringLiteral("btnOutputDir"));
        splitter_6->addWidget(btnOutputDir);
        scrollArea->setWidget(scrollAreaWidgetContents);

        verticalLayout_2->addWidget(scrollArea);


        retranslateUi(frmSetting);
        QObject::connect(btnTrain, SIGNAL(clicked()), frmSetting, SLOT(onTrain()));
        QObject::connect(btnSave, SIGNAL(clicked()), frmSetting, SLOT(onSave()));
        QObject::connect(txtCfgPath, SIGNAL(textChanged(QString)), frmSetting, SLOT(textChange(QString)));
        QObject::connect(txtDataPth, SIGNAL(textChanged(QString)), frmSetting, SLOT(textChange(QString)));
        QObject::connect(txtSavePath, SIGNAL(textChanged(QString)), frmSetting, SLOT(textChange(QString)));
        QObject::connect(txtSmpPath, SIGNAL(textChanged(QString)), frmSetting, SLOT(textChange(QString)));
        QObject::connect(txtOutputDir, SIGNAL(textChanged(QString)), frmSetting, SLOT(textChange(QString)));

        QMetaObject::connectSlotsByName(frmSetting);
    } // setupUi

    void retranslateUi(QDialog *frmSetting)
    {
        frmSetting->setWindowTitle(QApplication::translate("frmSetting", "frmSetting", nullptr));
        groupCfgPath->setTitle(QApplication::translate("frmSetting", "Config Path", nullptr));
        btnCfgSelect->setText(QApplication::translate("frmSetting", "Select", nullptr));
        groupSmpPath->setTitle(QApplication::translate("frmSetting", "Sample Folder", nullptr));
        btnSmpSelect->setText(QApplication::translate("frmSetting", "Select", nullptr));
        groupStudyData->setTitle(QApplication::translate("frmSetting", "KNN Data Path", nullptr));
        btnDataSelect->setText(QApplication::translate("frmSetting", "Select", nullptr));
        groupSaveFolder->setTitle(QApplication::translate("frmSetting", "Save Picure to", nullptr));
        btnSaveSelect->setText(QApplication::translate("frmSetting", "Select", nullptr));
        btnSave->setText(QApplication::translate("frmSetting", "Save", nullptr));
        btnTrain->setText(QApplication::translate("frmSetting", "Retrain", nullptr));
        groupOutputDir->setTitle(QApplication::translate("frmSetting", "Output Directory", nullptr));
        btnOutputDir->setText(QApplication::translate("frmSetting", "Select", nullptr));
    } // retranslateUi

};

namespace Ui {
    class frmSetting: public Ui_frmSetting {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FRMSETTING_H
