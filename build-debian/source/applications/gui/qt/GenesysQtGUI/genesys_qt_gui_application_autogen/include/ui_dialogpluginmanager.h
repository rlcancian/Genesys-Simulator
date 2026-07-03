/********************************************************************************
** Form generated from reading UI file 'dialogpluginmanager.ui'
**
** Created by: Qt User Interface Compiler version 6.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOGPLUGINMANAGER_H
#define UI_DIALOGPLUGINMANAGER_H

#include <QtCore/QVariant>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_DialogPluginManager
{
public:
    QVBoxLayout *verticalLayout;
    QGroupBox *groupBoxAutoload;
    QGridLayout *gridLayoutAutoload;
    QLabel *labelAutoloadFilename;
    QLineEdit *lineEditAutoloadFilename;
    QPushButton *pushButtonBrowseAutoload;
    QCheckBox *checkBoxFallbackDiscovery;
    QPushButton *pushButtonAutoLoadNow;
    QWidget *layoutWidgetPlugins;
    QVBoxLayout *verticalLayoutPlugins;
    QTabWidget *tabWidgetPluginTables;
    QWidget *tabLoadedPlugins;
    QVBoxLayout *verticalLayoutLoadedPlugins;
    QTableWidget *tableWidgetPlugins;
    QWidget *tabPluginIssues;
    QVBoxLayout *verticalLayoutPluginIssues;
    QTableWidget *tableWidgetPluginIssues;
    QHBoxLayout *horizontalLayoutActions;
    QPushButton *pushButtonCheck;
    QPushButton *pushButtonInsert;
    QPushButton *pushButtonResolveSelected;
    QPushButton *pushButtonRemove;
    QPushButton *pushButtonRefresh;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *DialogPluginManager)
    {
        if (DialogPluginManager->objectName().isEmpty())
            DialogPluginManager->setObjectName("DialogPluginManager");
        DialogPluginManager->setWindowModality(Qt::WindowModal);
        DialogPluginManager->resize(920, 620);
        verticalLayout = new QVBoxLayout(DialogPluginManager);
        verticalLayout->setObjectName("verticalLayout");
        groupBoxAutoload = new QGroupBox(DialogPluginManager);
        groupBoxAutoload->setObjectName("groupBoxAutoload");
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(groupBoxAutoload->sizePolicy().hasHeightForWidth());
        groupBoxAutoload->setSizePolicy(sizePolicy);
        gridLayoutAutoload = new QGridLayout(groupBoxAutoload);
        gridLayoutAutoload->setObjectName("gridLayoutAutoload");
        labelAutoloadFilename = new QLabel(groupBoxAutoload);
        labelAutoloadFilename->setObjectName("labelAutoloadFilename");

        gridLayoutAutoload->addWidget(labelAutoloadFilename, 0, 0, 1, 1);

        lineEditAutoloadFilename = new QLineEdit(groupBoxAutoload);
        lineEditAutoloadFilename->setObjectName("lineEditAutoloadFilename");

        gridLayoutAutoload->addWidget(lineEditAutoloadFilename, 0, 1, 1, 1);

        pushButtonBrowseAutoload = new QPushButton(groupBoxAutoload);
        pushButtonBrowseAutoload->setObjectName("pushButtonBrowseAutoload");

        gridLayoutAutoload->addWidget(pushButtonBrowseAutoload, 0, 2, 1, 1);

        checkBoxFallbackDiscovery = new QCheckBox(groupBoxAutoload);
        checkBoxFallbackDiscovery->setObjectName("checkBoxFallbackDiscovery");
        checkBoxFallbackDiscovery->setChecked(true);

        gridLayoutAutoload->addWidget(checkBoxFallbackDiscovery, 1, 1, 1, 1);

        pushButtonAutoLoadNow = new QPushButton(groupBoxAutoload);
        pushButtonAutoLoadNow->setObjectName("pushButtonAutoLoadNow");

        gridLayoutAutoload->addWidget(pushButtonAutoLoadNow, 1, 2, 1, 1);


        verticalLayout->addWidget(groupBoxAutoload);

        layoutWidgetPlugins = new QWidget(DialogPluginManager);
        layoutWidgetPlugins->setObjectName("layoutWidgetPlugins");
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(1);
        sizePolicy1.setHeightForWidth(layoutWidgetPlugins->sizePolicy().hasHeightForWidth());
        layoutWidgetPlugins->setSizePolicy(sizePolicy1);
        verticalLayoutPlugins = new QVBoxLayout(layoutWidgetPlugins);
        verticalLayoutPlugins->setObjectName("verticalLayoutPlugins");
        verticalLayoutPlugins->setContentsMargins(0, 0, 0, 0);
        tabWidgetPluginTables = new QTabWidget(layoutWidgetPlugins);
        tabWidgetPluginTables->setObjectName("tabWidgetPluginTables");
        sizePolicy1.setHeightForWidth(tabWidgetPluginTables->sizePolicy().hasHeightForWidth());
        tabWidgetPluginTables->setSizePolicy(sizePolicy1);
        tabLoadedPlugins = new QWidget();
        tabLoadedPlugins->setObjectName("tabLoadedPlugins");
        verticalLayoutLoadedPlugins = new QVBoxLayout(tabLoadedPlugins);
        verticalLayoutLoadedPlugins->setObjectName("verticalLayoutLoadedPlugins");
        tableWidgetPlugins = new QTableWidget(tabLoadedPlugins);
        tableWidgetPlugins->setObjectName("tableWidgetPlugins");
        tableWidgetPlugins->setEditTriggers(QAbstractItemView::NoEditTriggers);
        tableWidgetPlugins->setAlternatingRowColors(true);
        tableWidgetPlugins->setSelectionMode(QAbstractItemView::SingleSelection);
        tableWidgetPlugins->setSelectionBehavior(QAbstractItemView::SelectRows);
        tableWidgetPlugins->setSortingEnabled(true);

        verticalLayoutLoadedPlugins->addWidget(tableWidgetPlugins);

        tabWidgetPluginTables->addTab(tabLoadedPlugins, QString());
        tabPluginIssues = new QWidget();
        tabPluginIssues->setObjectName("tabPluginIssues");
        verticalLayoutPluginIssues = new QVBoxLayout(tabPluginIssues);
        verticalLayoutPluginIssues->setObjectName("verticalLayoutPluginIssues");
        tableWidgetPluginIssues = new QTableWidget(tabPluginIssues);
        tableWidgetPluginIssues->setObjectName("tableWidgetPluginIssues");
        tableWidgetPluginIssues->setEditTriggers(QAbstractItemView::NoEditTriggers);
        tableWidgetPluginIssues->setAlternatingRowColors(true);
        tableWidgetPluginIssues->setSelectionMode(QAbstractItemView::SingleSelection);
        tableWidgetPluginIssues->setSelectionBehavior(QAbstractItemView::SelectRows);

        verticalLayoutPluginIssues->addWidget(tableWidgetPluginIssues);

        tabWidgetPluginTables->addTab(tabPluginIssues, QString());

        verticalLayoutPlugins->addWidget(tabWidgetPluginTables);

        horizontalLayoutActions = new QHBoxLayout();
        horizontalLayoutActions->setObjectName("horizontalLayoutActions");
        pushButtonCheck = new QPushButton(layoutWidgetPlugins);
        pushButtonCheck->setObjectName("pushButtonCheck");

        horizontalLayoutActions->addWidget(pushButtonCheck);

        pushButtonInsert = new QPushButton(layoutWidgetPlugins);
        pushButtonInsert->setObjectName("pushButtonInsert");

        horizontalLayoutActions->addWidget(pushButtonInsert);

        pushButtonResolveSelected = new QPushButton(layoutWidgetPlugins);
        pushButtonResolveSelected->setObjectName("pushButtonResolveSelected");

        horizontalLayoutActions->addWidget(pushButtonResolveSelected);

        pushButtonRemove = new QPushButton(layoutWidgetPlugins);
        pushButtonRemove->setObjectName("pushButtonRemove");

        horizontalLayoutActions->addWidget(pushButtonRemove);

        pushButtonRefresh = new QPushButton(layoutWidgetPlugins);
        pushButtonRefresh->setObjectName("pushButtonRefresh");

        horizontalLayoutActions->addWidget(pushButtonRefresh);


        verticalLayoutPlugins->addLayout(horizontalLayoutActions);


        verticalLayout->addWidget(layoutWidgetPlugins);

        buttonBox = new QDialogButtonBox(DialogPluginManager);
        buttonBox->setObjectName("buttonBox");
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Close);

        verticalLayout->addWidget(buttonBox);

#if QT_CONFIG(shortcut)
        labelAutoloadFilename->setBuddy(lineEditAutoloadFilename);
#endif // QT_CONFIG(shortcut)
        QWidget::setTabOrder(lineEditAutoloadFilename, pushButtonBrowseAutoload);
        QWidget::setTabOrder(pushButtonBrowseAutoload, checkBoxFallbackDiscovery);
        QWidget::setTabOrder(checkBoxFallbackDiscovery, pushButtonAutoLoadNow);
        QWidget::setTabOrder(pushButtonAutoLoadNow, tabWidgetPluginTables);
        QWidget::setTabOrder(tabWidgetPluginTables, tableWidgetPlugins);
        QWidget::setTabOrder(tableWidgetPlugins, tableWidgetPluginIssues);
        QWidget::setTabOrder(tableWidgetPluginIssues, pushButtonCheck);
        QWidget::setTabOrder(pushButtonCheck, pushButtonInsert);
        QWidget::setTabOrder(pushButtonInsert, pushButtonResolveSelected);
        QWidget::setTabOrder(pushButtonResolveSelected, pushButtonRemove);
        QWidget::setTabOrder(pushButtonRemove, pushButtonRefresh);

        retranslateUi(DialogPluginManager);
        QObject::connect(buttonBox, &QDialogButtonBox::rejected, DialogPluginManager, qOverload<>(&QDialog::reject));

        tabWidgetPluginTables->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(DialogPluginManager);
    } // setupUi

    void retranslateUi(QDialog *DialogPluginManager)
    {
        DialogPluginManager->setWindowTitle(QCoreApplication::translate("DialogPluginManager", "Plugin Manager", nullptr));
        groupBoxAutoload->setTitle(QCoreApplication::translate("DialogPluginManager", "Auto Load", nullptr));
        labelAutoloadFilename->setText(QCoreApplication::translate("DialogPluginManager", "Plugin list file:", nullptr));
        lineEditAutoloadFilename->setText(QCoreApplication::translate("DialogPluginManager", "autoloadplugins.txt", nullptr));
        pushButtonBrowseAutoload->setText(QCoreApplication::translate("DialogPluginManager", "Browse...", nullptr));
        checkBoxFallbackDiscovery->setText(QCoreApplication::translate("DialogPluginManager", "Search for plugins if file is missing", nullptr));
        pushButtonAutoLoadNow->setText(QCoreApplication::translate("DialogPluginManager", "Auto Load Now", nullptr));
        tabWidgetPluginTables->setTabText(tabWidgetPluginTables->indexOf(tabLoadedPlugins), QCoreApplication::translate("DialogPluginManager", "Loaded plugins", nullptr));
        tabWidgetPluginTables->setTabText(tabWidgetPluginTables->indexOf(tabPluginIssues), QCoreApplication::translate("DialogPluginManager", "Plugins with problems", nullptr));
        pushButtonCheck->setText(QCoreApplication::translate("DialogPluginManager", "Check...", nullptr));
        pushButtonInsert->setText(QCoreApplication::translate("DialogPluginManager", "Insert...", nullptr));
        pushButtonResolveSelected->setText(QCoreApplication::translate("DialogPluginManager", "Resolve / Load Selected", nullptr));
        pushButtonRemove->setText(QCoreApplication::translate("DialogPluginManager", "Remove Selected", nullptr));
        pushButtonRefresh->setText(QCoreApplication::translate("DialogPluginManager", "Refresh", nullptr));
    } // retranslateUi

};

namespace Ui {
    class DialogPluginManager: public Ui_DialogPluginManager {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOGPLUGINMANAGER_H
