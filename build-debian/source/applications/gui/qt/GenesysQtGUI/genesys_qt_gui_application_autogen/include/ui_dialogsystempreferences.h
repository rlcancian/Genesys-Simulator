/********************************************************************************
** Form generated from reading UI file 'dialogsystempreferences.ui'
**
** Created by: Qt User Interface Compiler version 6.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOGSYSTEMPREFERENCES_H
#define UI_DIALOGSYSTEMPREFERENCES_H

#include <QtCore/QVariant>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_DialogSystemPreferences
{
public:
    QVBoxLayout *verticalLayout;
    QTabWidget *tabWidget;
    QWidget *tabGeneral;
    QVBoxLayout *verticalLayoutGeneral;
    QGroupBox *groupBoxStartup;
    QVBoxLayout *verticalLayoutStartup;
    QCheckBox *checkBoxStartMaximized;
    QGroupBox *groupBoxStartupModel;
    QGridLayout *gridLayoutStartupModel;
    QRadioButton *radioButtonStartNoModel;
    QRadioButton *radioButtonStartNewModel;
    QRadioButton *radioButtonStartLastModel;
    QRadioButton *radioButtonStartSpecificModel;
    QLineEdit *lineEditStartupModelFile;
    QPushButton *pushButtonBrowseStartupModel;
    QGroupBox *groupBoxRuntime;
    QFormLayout *formLayoutRuntime;
    QCheckBox *checkBoxAutoLoadPlugins;
    QCheckBox *checkBoxCheckSystemPackagesAtStart;
    QLabel *labelTraceLevel;
    QComboBox *comboBoxTraceLevel;
    QSpacerItem *verticalSpacerGeneral;
    QWidget *tabView;
    QVBoxLayout *verticalLayoutView;
    QGroupBox *groupBoxTheme;
    QFormLayout *formLayoutTheme;
    QLabel *labelTheme;
    QComboBox *comboBoxTheme;
    QLabel *labelInterfaceStyle;
    QComboBox *comboBoxInterfaceStyle;
    QLabel *labelFontPointSize;
    QSpinBox *spinBoxFontPointSize;
    QGroupBox *groupBoxDiagramTheme;
    QFormLayout *formLayoutDiagramTheme;
    QCheckBox *checkBoxDiagramUsesThemeColors;
    QLabel *labelRecentModelsLimit;
    QSpinBox *spinBoxRecentModelsLimit;
    QLabel *labelAutomaticPositioningStrategy;
    QComboBox *comboBoxAutomaticPositioningStrategy;
    QGroupBox *groupBoxConfigFile;
    QFormLayout *formLayoutConfig;
    QLabel *labelConfigPath;
    QLabel *labelConfigPathValue;
    QSpacerItem *verticalSpacerView;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *DialogSystemPreferences)
    {
        if (DialogSystemPreferences->objectName().isEmpty())
            DialogSystemPreferences->setObjectName("DialogSystemPreferences");
        DialogSystemPreferences->resize(680, 520);
        DialogSystemPreferences->setMinimumSize(QSize(560, 420));
        verticalLayout = new QVBoxLayout(DialogSystemPreferences);
        verticalLayout->setObjectName("verticalLayout");
        tabWidget = new QTabWidget(DialogSystemPreferences);
        tabWidget->setObjectName("tabWidget");
        tabGeneral = new QWidget();
        tabGeneral->setObjectName("tabGeneral");
        verticalLayoutGeneral = new QVBoxLayout(tabGeneral);
        verticalLayoutGeneral->setObjectName("verticalLayoutGeneral");
        groupBoxStartup = new QGroupBox(tabGeneral);
        groupBoxStartup->setObjectName("groupBoxStartup");
        verticalLayoutStartup = new QVBoxLayout(groupBoxStartup);
        verticalLayoutStartup->setObjectName("verticalLayoutStartup");
        checkBoxStartMaximized = new QCheckBox(groupBoxStartup);
        checkBoxStartMaximized->setObjectName("checkBoxStartMaximized");

        verticalLayoutStartup->addWidget(checkBoxStartMaximized);

        groupBoxStartupModel = new QGroupBox(groupBoxStartup);
        groupBoxStartupModel->setObjectName("groupBoxStartupModel");
        gridLayoutStartupModel = new QGridLayout(groupBoxStartupModel);
        gridLayoutStartupModel->setObjectName("gridLayoutStartupModel");
        radioButtonStartNoModel = new QRadioButton(groupBoxStartupModel);
        radioButtonStartNoModel->setObjectName("radioButtonStartNoModel");

        gridLayoutStartupModel->addWidget(radioButtonStartNoModel, 0, 0, 1, 3);

        radioButtonStartNewModel = new QRadioButton(groupBoxStartupModel);
        radioButtonStartNewModel->setObjectName("radioButtonStartNewModel");

        gridLayoutStartupModel->addWidget(radioButtonStartNewModel, 1, 0, 1, 3);

        radioButtonStartLastModel = new QRadioButton(groupBoxStartupModel);
        radioButtonStartLastModel->setObjectName("radioButtonStartLastModel");

        gridLayoutStartupModel->addWidget(radioButtonStartLastModel, 2, 0, 1, 3);

        radioButtonStartSpecificModel = new QRadioButton(groupBoxStartupModel);
        radioButtonStartSpecificModel->setObjectName("radioButtonStartSpecificModel");

        gridLayoutStartupModel->addWidget(radioButtonStartSpecificModel, 3, 0, 1, 1);

        lineEditStartupModelFile = new QLineEdit(groupBoxStartupModel);
        lineEditStartupModelFile->setObjectName("lineEditStartupModelFile");

        gridLayoutStartupModel->addWidget(lineEditStartupModelFile, 3, 1, 1, 1);

        pushButtonBrowseStartupModel = new QPushButton(groupBoxStartupModel);
        pushButtonBrowseStartupModel->setObjectName("pushButtonBrowseStartupModel");

        gridLayoutStartupModel->addWidget(pushButtonBrowseStartupModel, 3, 2, 1, 1);


        verticalLayoutStartup->addWidget(groupBoxStartupModel);


        verticalLayoutGeneral->addWidget(groupBoxStartup);

        groupBoxRuntime = new QGroupBox(tabGeneral);
        groupBoxRuntime->setObjectName("groupBoxRuntime");
        formLayoutRuntime = new QFormLayout(groupBoxRuntime);
        formLayoutRuntime->setObjectName("formLayoutRuntime");
        checkBoxAutoLoadPlugins = new QCheckBox(groupBoxRuntime);
        checkBoxAutoLoadPlugins->setObjectName("checkBoxAutoLoadPlugins");

        formLayoutRuntime->setWidget(0, QFormLayout::SpanningRole, checkBoxAutoLoadPlugins);

        checkBoxCheckSystemPackagesAtStart = new QCheckBox(groupBoxRuntime);
        checkBoxCheckSystemPackagesAtStart->setObjectName("checkBoxCheckSystemPackagesAtStart");

        formLayoutRuntime->setWidget(1, QFormLayout::SpanningRole, checkBoxCheckSystemPackagesAtStart);

        labelTraceLevel = new QLabel(groupBoxRuntime);
        labelTraceLevel->setObjectName("labelTraceLevel");

        formLayoutRuntime->setWidget(2, QFormLayout::LabelRole, labelTraceLevel);

        comboBoxTraceLevel = new QComboBox(groupBoxRuntime);
        comboBoxTraceLevel->setObjectName("comboBoxTraceLevel");

        formLayoutRuntime->setWidget(2, QFormLayout::FieldRole, comboBoxTraceLevel);


        verticalLayoutGeneral->addWidget(groupBoxRuntime);

        verticalSpacerGeneral = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayoutGeneral->addItem(verticalSpacerGeneral);

        tabWidget->addTab(tabGeneral, QString());
        tabView = new QWidget();
        tabView->setObjectName("tabView");
        verticalLayoutView = new QVBoxLayout(tabView);
        verticalLayoutView->setObjectName("verticalLayoutView");
        groupBoxTheme = new QGroupBox(tabView);
        groupBoxTheme->setObjectName("groupBoxTheme");
        formLayoutTheme = new QFormLayout(groupBoxTheme);
        formLayoutTheme->setObjectName("formLayoutTheme");
        labelTheme = new QLabel(groupBoxTheme);
        labelTheme->setObjectName("labelTheme");

        formLayoutTheme->setWidget(0, QFormLayout::LabelRole, labelTheme);

        comboBoxTheme = new QComboBox(groupBoxTheme);
        comboBoxTheme->addItem(QString());
        comboBoxTheme->addItem(QString());
        comboBoxTheme->setObjectName("comboBoxTheme");

        formLayoutTheme->setWidget(0, QFormLayout::FieldRole, comboBoxTheme);

        labelInterfaceStyle = new QLabel(groupBoxTheme);
        labelInterfaceStyle->setObjectName("labelInterfaceStyle");

        formLayoutTheme->setWidget(1, QFormLayout::LabelRole, labelInterfaceStyle);

        comboBoxInterfaceStyle = new QComboBox(groupBoxTheme);
        comboBoxInterfaceStyle->addItem(QString());
        comboBoxInterfaceStyle->addItem(QString());
        comboBoxInterfaceStyle->setObjectName("comboBoxInterfaceStyle");

        formLayoutTheme->setWidget(1, QFormLayout::FieldRole, comboBoxInterfaceStyle);

        labelFontPointSize = new QLabel(groupBoxTheme);
        labelFontPointSize->setObjectName("labelFontPointSize");

        formLayoutTheme->setWidget(2, QFormLayout::LabelRole, labelFontPointSize);

        spinBoxFontPointSize = new QSpinBox(groupBoxTheme);
        spinBoxFontPointSize->setObjectName("spinBoxFontPointSize");
        spinBoxFontPointSize->setMinimum(0);
        spinBoxFontPointSize->setMaximum(24);

        formLayoutTheme->setWidget(2, QFormLayout::FieldRole, spinBoxFontPointSize);


        verticalLayoutView->addWidget(groupBoxTheme);

        groupBoxDiagramTheme = new QGroupBox(tabView);
        groupBoxDiagramTheme->setObjectName("groupBoxDiagramTheme");
        formLayoutDiagramTheme = new QFormLayout(groupBoxDiagramTheme);
        formLayoutDiagramTheme->setObjectName("formLayoutDiagramTheme");
        checkBoxDiagramUsesThemeColors = new QCheckBox(groupBoxDiagramTheme);
        checkBoxDiagramUsesThemeColors->setObjectName("checkBoxDiagramUsesThemeColors");

        formLayoutDiagramTheme->setWidget(0, QFormLayout::SpanningRole, checkBoxDiagramUsesThemeColors);

        labelRecentModelsLimit = new QLabel(groupBoxDiagramTheme);
        labelRecentModelsLimit->setObjectName("labelRecentModelsLimit");

        formLayoutDiagramTheme->setWidget(1, QFormLayout::LabelRole, labelRecentModelsLimit);

        spinBoxRecentModelsLimit = new QSpinBox(groupBoxDiagramTheme);
        spinBoxRecentModelsLimit->setObjectName("spinBoxRecentModelsLimit");
        spinBoxRecentModelsLimit->setMinimum(1);
        spinBoxRecentModelsLimit->setMaximum(50);
        spinBoxRecentModelsLimit->setValue(10);

        formLayoutDiagramTheme->setWidget(1, QFormLayout::FieldRole, spinBoxRecentModelsLimit);

        labelAutomaticPositioningStrategy = new QLabel(groupBoxDiagramTheme);
        labelAutomaticPositioningStrategy->setObjectName("labelAutomaticPositioningStrategy");

        formLayoutDiagramTheme->setWidget(2, QFormLayout::LabelRole, labelAutomaticPositioningStrategy);

        comboBoxAutomaticPositioningStrategy = new QComboBox(groupBoxDiagramTheme);
        comboBoxAutomaticPositioningStrategy->addItem(QString());
        comboBoxAutomaticPositioningStrategy->addItem(QString());
        comboBoxAutomaticPositioningStrategy->setObjectName("comboBoxAutomaticPositioningStrategy");

        formLayoutDiagramTheme->setWidget(2, QFormLayout::FieldRole, comboBoxAutomaticPositioningStrategy);


        verticalLayoutView->addWidget(groupBoxDiagramTheme);

        groupBoxConfigFile = new QGroupBox(tabView);
        groupBoxConfigFile->setObjectName("groupBoxConfigFile");
        formLayoutConfig = new QFormLayout(groupBoxConfigFile);
        formLayoutConfig->setObjectName("formLayoutConfig");
        labelConfigPath = new QLabel(groupBoxConfigFile);
        labelConfigPath->setObjectName("labelConfigPath");

        formLayoutConfig->setWidget(0, QFormLayout::LabelRole, labelConfigPath);

        labelConfigPathValue = new QLabel(groupBoxConfigFile);
        labelConfigPathValue->setObjectName("labelConfigPathValue");
        labelConfigPathValue->setTextInteractionFlags(Qt::TextSelectableByMouse);

        formLayoutConfig->setWidget(0, QFormLayout::FieldRole, labelConfigPathValue);


        verticalLayoutView->addWidget(groupBoxConfigFile);

        verticalSpacerView = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayoutView->addItem(verticalSpacerView);

        tabWidget->addTab(tabView, QString());

        verticalLayout->addWidget(tabWidget);

        buttonBox = new QDialogButtonBox(DialogSystemPreferences);
        buttonBox->setObjectName("buttonBox");
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(DialogSystemPreferences);
        QObject::connect(buttonBox, &QDialogButtonBox::rejected, DialogSystemPreferences, qOverload<>(&QDialog::reject));

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(DialogSystemPreferences);
    } // setupUi

    void retranslateUi(QDialog *DialogSystemPreferences)
    {
        DialogSystemPreferences->setWindowTitle(QCoreApplication::translate("DialogSystemPreferences", "System Preferences", nullptr));
        groupBoxStartup->setTitle(QCoreApplication::translate("DialogSystemPreferences", "Startup", nullptr));
        checkBoxStartMaximized->setText(QCoreApplication::translate("DialogSystemPreferences", "Start main window maximized", nullptr));
        groupBoxStartupModel->setTitle(QCoreApplication::translate("DialogSystemPreferences", "Model at startup", nullptr));
        radioButtonStartNoModel->setText(QCoreApplication::translate("DialogSystemPreferences", "Open without a model", nullptr));
        radioButtonStartNewModel->setText(QCoreApplication::translate("DialogSystemPreferences", "Create a new model automatically", nullptr));
        radioButtonStartLastModel->setText(QCoreApplication::translate("DialogSystemPreferences", "Open last successfully used model", nullptr));
        radioButtonStartSpecificModel->setText(QCoreApplication::translate("DialogSystemPreferences", "Open this model file", nullptr));
        pushButtonBrowseStartupModel->setText(QCoreApplication::translate("DialogSystemPreferences", "Browse...", nullptr));
        groupBoxRuntime->setTitle(QCoreApplication::translate("DialogSystemPreferences", "Runtime and diagnostics", nullptr));
        checkBoxAutoLoadPlugins->setText(QCoreApplication::translate("DialogSystemPreferences", "Automatically load configured plugins at startup", nullptr));
        checkBoxCheckSystemPackagesAtStart->setText(QCoreApplication::translate("DialogSystemPreferences", "Check required system packages at startup", nullptr));
        labelTraceLevel->setText(QCoreApplication::translate("DialogSystemPreferences", "Trace level", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tabGeneral), QCoreApplication::translate("DialogSystemPreferences", "General", nullptr));
        groupBoxTheme->setTitle(QCoreApplication::translate("DialogSystemPreferences", "Application appearance", nullptr));
        labelTheme->setText(QCoreApplication::translate("DialogSystemPreferences", "Theme", nullptr));
        comboBoxTheme->setItemText(0, QCoreApplication::translate("DialogSystemPreferences", "Light", nullptr));
        comboBoxTheme->setItemText(1, QCoreApplication::translate("DialogSystemPreferences", "Dark", nullptr));

        labelInterfaceStyle->setText(QCoreApplication::translate("DialogSystemPreferences", "Interface style", nullptr));
        comboBoxInterfaceStyle->setItemText(0, QCoreApplication::translate("DialogSystemPreferences", "Classic Desktop", nullptr));
        comboBoxInterfaceStyle->setItemText(1, QCoreApplication::translate("DialogSystemPreferences", "Modern Fusion", nullptr));

        labelFontPointSize->setText(QCoreApplication::translate("DialogSystemPreferences", "Font point size", nullptr));
        spinBoxFontPointSize->setSpecialValueText(QCoreApplication::translate("DialogSystemPreferences", "System default", nullptr));
        groupBoxDiagramTheme->setTitle(QCoreApplication::translate("DialogSystemPreferences", "Model diagram", nullptr));
        checkBoxDiagramUsesThemeColors->setText(QCoreApplication::translate("DialogSystemPreferences", "Apply theme colors to canvas background and grid", nullptr));
        labelRecentModelsLimit->setText(QCoreApplication::translate("DialogSystemPreferences", "Recent files in menu", nullptr));
        labelAutomaticPositioningStrategy->setText(QCoreApplication::translate("DialogSystemPreferences", "Automatic positioning strategy", nullptr));
        comboBoxAutomaticPositioningStrategy->setItemText(0, QCoreApplication::translate("DialogSystemPreferences", "Centered (recommended)", nullptr));
        comboBoxAutomaticPositioningStrategy->setItemText(1, QCoreApplication::translate("DialogSystemPreferences", "Legacy (top-left)", nullptr));

        groupBoxConfigFile->setTitle(QCoreApplication::translate("DialogSystemPreferences", "Runtime configuration", nullptr));
        labelConfigPath->setText(QCoreApplication::translate("DialogSystemPreferences", "JSON file", nullptr));
        labelConfigPathValue->setText(QString());
        tabWidget->setTabText(tabWidget->indexOf(tabView), QCoreApplication::translate("DialogSystemPreferences", "View", nullptr));
    } // retranslateUi

};

namespace Ui {
    class DialogSystemPreferences: public Ui_DialogSystemPreferences {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOGSYSTEMPREFERENCES_H
