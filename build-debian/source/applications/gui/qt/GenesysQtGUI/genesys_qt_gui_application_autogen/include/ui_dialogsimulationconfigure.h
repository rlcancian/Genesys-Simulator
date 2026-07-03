/********************************************************************************
** Form generated from reading UI file 'dialogsimulationconfigure.ui'
**
** Created by: Qt User Interface Compiler version 6.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOGSIMULATIONCONFIGURE_H
#define UI_DIALOGSIMULATIONCONFIGURE_H

#include <QtCore/QVariant>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_DialogSimulationConfigure
{
public:
    QVBoxLayout *verticalLayout;
    QTabWidget *tabWidgetSimulationConfigure;
    QWidget *tabModelSimulation;
    QVBoxLayout *verticalLayoutModelSimulation;
    QFormLayout *formLayout;
    QLabel *labelNumberOfReplications;
    QSpinBox *spinBoxNumberOfReplications;
    QLabel *labelReplicationBaseTimeUnit;
    QComboBox *comboBoxReplicationBaseTimeUnit;
    QLabel *labelReplicationLength;
    QHBoxLayout *horizontalLayoutReplicationLength;
    QDoubleSpinBox *doubleSpinBoxReplicationLength;
    QComboBox *comboBoxReplicationLengthTimeUnit;
    QLabel *labelWarmUpPeriod;
    QHBoxLayout *horizontalLayoutWarmUpPeriod;
    QDoubleSpinBox *doubleSpinBoxWarmUpPeriod;
    QComboBox *comboBoxWarmUpPeriodTimeUnit;
    QLabel *labelTerminatingCondition;
    QPlainTextEdit *plainTextTerminatingCondition;
    QHBoxLayout *horizontalLayoutInitializers;
    QCheckBox *checkBoxInitializeSystem;
    QCheckBox *checkBoxInitializeStatistics;
    QHBoxLayout *horizontalLayoutExecution;
    QCheckBox *checkBoxStepByStep;
    QCheckBox *checkBoxPauseOnEvent;
    QCheckBox *checkBoxPauseOnReplication;
    QSpacerItem *verticalSpacerModelSimulation;
    QWidget *tabSimulationReport;
    QVBoxLayout *verticalLayoutSimulationReport;
    QLabel *labelSimulationReporterStatus;
    QLabel *labelSimulationReporterPlaceholder;
    QSpacerItem *verticalSpacerSimulationReport;
    QWidget *tabParallelization;
    QVBoxLayout *verticalLayoutParallelization;
    QLabel *labelParallelizationIntro;
    QGroupBox *groupBoxLocalParallelization;
    QFormLayout *formLayoutLocalParallelization;
    QCheckBox *checkBoxParallelizationEnabled;
    QLabel *labelParallelizationThreads;
    QSpinBox *spinBoxParallelizationThreads;
    QLabel *labelParallelizationBatchSize;
    QSpinBox *spinBoxParallelizationBatchSize;
    QGroupBox *groupBoxDistributedParallelization;
    QFormLayout *formLayoutDistributedParallelization;
    QCheckBox *checkBoxDistributedParallelizationEnabled;
    QLabel *labelDistributedCoordinatorUrl;
    QLineEdit *lineEditDistributedCoordinatorUrl;
    QLabel *labelDistributedToken;
    QLineEdit *lineEditDistributedToken;
    QPushButton *pushButtonConfigureDistributedParallelization;
    QLabel *labelParallelizationStatus;
    QSpacerItem *verticalSpacerParallelization;
    QWidget *tabExperimentManager;
    QVBoxLayout *verticalLayoutExperimentManager;
    QLabel *labelExperimentManagerStatus;
    QLabel *labelExperimentManagerPlaceholder;
    QSpacerItem *verticalSpacerExperimentManager;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *DialogSimulationConfigure)
    {
        if (DialogSimulationConfigure->objectName().isEmpty())
            DialogSimulationConfigure->setObjectName("DialogSimulationConfigure");
        DialogSimulationConfigure->setWindowModality(Qt::WindowModal);
        DialogSimulationConfigure->resize(620, 500);
        verticalLayout = new QVBoxLayout(DialogSimulationConfigure);
        verticalLayout->setObjectName("verticalLayout");
        tabWidgetSimulationConfigure = new QTabWidget(DialogSimulationConfigure);
        tabWidgetSimulationConfigure->setObjectName("tabWidgetSimulationConfigure");
        tabModelSimulation = new QWidget();
        tabModelSimulation->setObjectName("tabModelSimulation");
        verticalLayoutModelSimulation = new QVBoxLayout(tabModelSimulation);
        verticalLayoutModelSimulation->setObjectName("verticalLayoutModelSimulation");
        formLayout = new QFormLayout();
        formLayout->setObjectName("formLayout");
        formLayout->setContentsMargins(0, -1, -1, 2);
        labelNumberOfReplications = new QLabel(tabModelSimulation);
        labelNumberOfReplications->setObjectName("labelNumberOfReplications");

        formLayout->setWidget(0, QFormLayout::LabelRole, labelNumberOfReplications);

        spinBoxNumberOfReplications = new QSpinBox(tabModelSimulation);
        spinBoxNumberOfReplications->setObjectName("spinBoxNumberOfReplications");
        spinBoxNumberOfReplications->setMinimum(1);
        spinBoxNumberOfReplications->setMaximum(2147483647);
        spinBoxNumberOfReplications->setValue(1);

        formLayout->setWidget(0, QFormLayout::FieldRole, spinBoxNumberOfReplications);

        labelReplicationBaseTimeUnit = new QLabel(tabModelSimulation);
        labelReplicationBaseTimeUnit->setObjectName("labelReplicationBaseTimeUnit");

        formLayout->setWidget(1, QFormLayout::LabelRole, labelReplicationBaseTimeUnit);

        comboBoxReplicationBaseTimeUnit = new QComboBox(tabModelSimulation);
        comboBoxReplicationBaseTimeUnit->setObjectName("comboBoxReplicationBaseTimeUnit");

        formLayout->setWidget(1, QFormLayout::FieldRole, comboBoxReplicationBaseTimeUnit);

        labelReplicationLength = new QLabel(tabModelSimulation);
        labelReplicationLength->setObjectName("labelReplicationLength");

        formLayout->setWidget(2, QFormLayout::LabelRole, labelReplicationLength);

        horizontalLayoutReplicationLength = new QHBoxLayout();
        horizontalLayoutReplicationLength->setObjectName("horizontalLayoutReplicationLength");
        doubleSpinBoxReplicationLength = new QDoubleSpinBox(tabModelSimulation);
        doubleSpinBoxReplicationLength->setObjectName("doubleSpinBoxReplicationLength");
        doubleSpinBoxReplicationLength->setDecimals(12);
        doubleSpinBoxReplicationLength->setMinimum(0.000001000000000);
        doubleSpinBoxReplicationLength->setMaximum(1000000000000.000000000000000);
        doubleSpinBoxReplicationLength->setSingleStep(1.000000000000000);

        horizontalLayoutReplicationLength->addWidget(doubleSpinBoxReplicationLength);

        comboBoxReplicationLengthTimeUnit = new QComboBox(tabModelSimulation);
        comboBoxReplicationLengthTimeUnit->setObjectName("comboBoxReplicationLengthTimeUnit");

        horizontalLayoutReplicationLength->addWidget(comboBoxReplicationLengthTimeUnit);


        formLayout->setLayout(2, QFormLayout::FieldRole, horizontalLayoutReplicationLength);

        labelWarmUpPeriod = new QLabel(tabModelSimulation);
        labelWarmUpPeriod->setObjectName("labelWarmUpPeriod");

        formLayout->setWidget(3, QFormLayout::LabelRole, labelWarmUpPeriod);

        horizontalLayoutWarmUpPeriod = new QHBoxLayout();
        horizontalLayoutWarmUpPeriod->setObjectName("horizontalLayoutWarmUpPeriod");
        doubleSpinBoxWarmUpPeriod = new QDoubleSpinBox(tabModelSimulation);
        doubleSpinBoxWarmUpPeriod->setObjectName("doubleSpinBoxWarmUpPeriod");
        doubleSpinBoxWarmUpPeriod->setDecimals(12);
        doubleSpinBoxWarmUpPeriod->setMinimum(0.000000000000000);
        doubleSpinBoxWarmUpPeriod->setMaximum(1000000000000.000000000000000);
        doubleSpinBoxWarmUpPeriod->setSingleStep(1.000000000000000);

        horizontalLayoutWarmUpPeriod->addWidget(doubleSpinBoxWarmUpPeriod);

        comboBoxWarmUpPeriodTimeUnit = new QComboBox(tabModelSimulation);
        comboBoxWarmUpPeriodTimeUnit->setObjectName("comboBoxWarmUpPeriodTimeUnit");

        horizontalLayoutWarmUpPeriod->addWidget(comboBoxWarmUpPeriodTimeUnit);


        formLayout->setLayout(3, QFormLayout::FieldRole, horizontalLayoutWarmUpPeriod);

        labelTerminatingCondition = new QLabel(tabModelSimulation);
        labelTerminatingCondition->setObjectName("labelTerminatingCondition");

        formLayout->setWidget(4, QFormLayout::LabelRole, labelTerminatingCondition);

        plainTextTerminatingCondition = new QPlainTextEdit(tabModelSimulation);
        plainTextTerminatingCondition->setObjectName("plainTextTerminatingCondition");

        formLayout->setWidget(4, QFormLayout::FieldRole, plainTextTerminatingCondition);


        verticalLayoutModelSimulation->addLayout(formLayout);

        horizontalLayoutInitializers = new QHBoxLayout();
        horizontalLayoutInitializers->setObjectName("horizontalLayoutInitializers");
        checkBoxInitializeSystem = new QCheckBox(tabModelSimulation);
        checkBoxInitializeSystem->setObjectName("checkBoxInitializeSystem");

        horizontalLayoutInitializers->addWidget(checkBoxInitializeSystem);

        checkBoxInitializeStatistics = new QCheckBox(tabModelSimulation);
        checkBoxInitializeStatistics->setObjectName("checkBoxInitializeStatistics");

        horizontalLayoutInitializers->addWidget(checkBoxInitializeStatistics);


        verticalLayoutModelSimulation->addLayout(horizontalLayoutInitializers);

        horizontalLayoutExecution = new QHBoxLayout();
        horizontalLayoutExecution->setObjectName("horizontalLayoutExecution");
        checkBoxStepByStep = new QCheckBox(tabModelSimulation);
        checkBoxStepByStep->setObjectName("checkBoxStepByStep");

        horizontalLayoutExecution->addWidget(checkBoxStepByStep);

        checkBoxPauseOnEvent = new QCheckBox(tabModelSimulation);
        checkBoxPauseOnEvent->setObjectName("checkBoxPauseOnEvent");

        horizontalLayoutExecution->addWidget(checkBoxPauseOnEvent);

        checkBoxPauseOnReplication = new QCheckBox(tabModelSimulation);
        checkBoxPauseOnReplication->setObjectName("checkBoxPauseOnReplication");

        horizontalLayoutExecution->addWidget(checkBoxPauseOnReplication);


        verticalLayoutModelSimulation->addLayout(horizontalLayoutExecution);

        verticalSpacerModelSimulation = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayoutModelSimulation->addItem(verticalSpacerModelSimulation);

        tabWidgetSimulationConfigure->addTab(tabModelSimulation, QString());
        tabSimulationReport = new QWidget();
        tabSimulationReport->setObjectName("tabSimulationReport");
        verticalLayoutSimulationReport = new QVBoxLayout(tabSimulationReport);
        verticalLayoutSimulationReport->setObjectName("verticalLayoutSimulationReport");
        labelSimulationReporterStatus = new QLabel(tabSimulationReport);
        labelSimulationReporterStatus->setObjectName("labelSimulationReporterStatus");
        labelSimulationReporterStatus->setWordWrap(true);

        verticalLayoutSimulationReport->addWidget(labelSimulationReporterStatus);

        labelSimulationReporterPlaceholder = new QLabel(tabSimulationReport);
        labelSimulationReporterPlaceholder->setObjectName("labelSimulationReporterPlaceholder");
        labelSimulationReporterPlaceholder->setWordWrap(true);

        verticalLayoutSimulationReport->addWidget(labelSimulationReporterPlaceholder);

        verticalSpacerSimulationReport = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayoutSimulationReport->addItem(verticalSpacerSimulationReport);

        tabWidgetSimulationConfigure->addTab(tabSimulationReport, QString());
        tabParallelization = new QWidget();
        tabParallelization->setObjectName("tabParallelization");
        verticalLayoutParallelization = new QVBoxLayout(tabParallelization);
        verticalLayoutParallelization->setObjectName("verticalLayoutParallelization");
        labelParallelizationIntro = new QLabel(tabParallelization);
        labelParallelizationIntro->setObjectName("labelParallelizationIntro");
        labelParallelizationIntro->setWordWrap(true);

        verticalLayoutParallelization->addWidget(labelParallelizationIntro);

        groupBoxLocalParallelization = new QGroupBox(tabParallelization);
        groupBoxLocalParallelization->setObjectName("groupBoxLocalParallelization");
        formLayoutLocalParallelization = new QFormLayout(groupBoxLocalParallelization);
        formLayoutLocalParallelization->setObjectName("formLayoutLocalParallelization");
        checkBoxParallelizationEnabled = new QCheckBox(groupBoxLocalParallelization);
        checkBoxParallelizationEnabled->setObjectName("checkBoxParallelizationEnabled");

        formLayoutLocalParallelization->setWidget(0, QFormLayout::SpanningRole, checkBoxParallelizationEnabled);

        labelParallelizationThreads = new QLabel(groupBoxLocalParallelization);
        labelParallelizationThreads->setObjectName("labelParallelizationThreads");

        formLayoutLocalParallelization->setWidget(1, QFormLayout::LabelRole, labelParallelizationThreads);

        spinBoxParallelizationThreads = new QSpinBox(groupBoxLocalParallelization);
        spinBoxParallelizationThreads->setObjectName("spinBoxParallelizationThreads");
        spinBoxParallelizationThreads->setMinimum(1);
        spinBoxParallelizationThreads->setMaximum(1024);
        spinBoxParallelizationThreads->setValue(1);

        formLayoutLocalParallelization->setWidget(1, QFormLayout::FieldRole, spinBoxParallelizationThreads);

        labelParallelizationBatchSize = new QLabel(groupBoxLocalParallelization);
        labelParallelizationBatchSize->setObjectName("labelParallelizationBatchSize");

        formLayoutLocalParallelization->setWidget(2, QFormLayout::LabelRole, labelParallelizationBatchSize);

        spinBoxParallelizationBatchSize = new QSpinBox(groupBoxLocalParallelization);
        spinBoxParallelizationBatchSize->setObjectName("spinBoxParallelizationBatchSize");
        spinBoxParallelizationBatchSize->setMinimum(1);
        spinBoxParallelizationBatchSize->setMaximum(1000000);
        spinBoxParallelizationBatchSize->setValue(100);

        formLayoutLocalParallelization->setWidget(2, QFormLayout::FieldRole, spinBoxParallelizationBatchSize);


        verticalLayoutParallelization->addWidget(groupBoxLocalParallelization);

        groupBoxDistributedParallelization = new QGroupBox(tabParallelization);
        groupBoxDistributedParallelization->setObjectName("groupBoxDistributedParallelization");
        formLayoutDistributedParallelization = new QFormLayout(groupBoxDistributedParallelization);
        formLayoutDistributedParallelization->setObjectName("formLayoutDistributedParallelization");
        checkBoxDistributedParallelizationEnabled = new QCheckBox(groupBoxDistributedParallelization);
        checkBoxDistributedParallelizationEnabled->setObjectName("checkBoxDistributedParallelizationEnabled");

        formLayoutDistributedParallelization->setWidget(0, QFormLayout::SpanningRole, checkBoxDistributedParallelizationEnabled);

        labelDistributedCoordinatorUrl = new QLabel(groupBoxDistributedParallelization);
        labelDistributedCoordinatorUrl->setObjectName("labelDistributedCoordinatorUrl");

        formLayoutDistributedParallelization->setWidget(1, QFormLayout::LabelRole, labelDistributedCoordinatorUrl);

        lineEditDistributedCoordinatorUrl = new QLineEdit(groupBoxDistributedParallelization);
        lineEditDistributedCoordinatorUrl->setObjectName("lineEditDistributedCoordinatorUrl");

        formLayoutDistributedParallelization->setWidget(1, QFormLayout::FieldRole, lineEditDistributedCoordinatorUrl);

        labelDistributedToken = new QLabel(groupBoxDistributedParallelization);
        labelDistributedToken->setObjectName("labelDistributedToken");

        formLayoutDistributedParallelization->setWidget(2, QFormLayout::LabelRole, labelDistributedToken);

        lineEditDistributedToken = new QLineEdit(groupBoxDistributedParallelization);
        lineEditDistributedToken->setObjectName("lineEditDistributedToken");
        lineEditDistributedToken->setEchoMode(QLineEdit::Password);

        formLayoutDistributedParallelization->setWidget(2, QFormLayout::FieldRole, lineEditDistributedToken);

        pushButtonConfigureDistributedParallelization = new QPushButton(groupBoxDistributedParallelization);
        pushButtonConfigureDistributedParallelization->setObjectName("pushButtonConfigureDistributedParallelization");

        formLayoutDistributedParallelization->setWidget(3, QFormLayout::SpanningRole, pushButtonConfigureDistributedParallelization);


        verticalLayoutParallelization->addWidget(groupBoxDistributedParallelization);

        labelParallelizationStatus = new QLabel(tabParallelization);
        labelParallelizationStatus->setObjectName("labelParallelizationStatus");
        labelParallelizationStatus->setWordWrap(true);

        verticalLayoutParallelization->addWidget(labelParallelizationStatus);

        verticalSpacerParallelization = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayoutParallelization->addItem(verticalSpacerParallelization);

        tabWidgetSimulationConfigure->addTab(tabParallelization, QString());
        tabExperimentManager = new QWidget();
        tabExperimentManager->setObjectName("tabExperimentManager");
        verticalLayoutExperimentManager = new QVBoxLayout(tabExperimentManager);
        verticalLayoutExperimentManager->setObjectName("verticalLayoutExperimentManager");
        labelExperimentManagerStatus = new QLabel(tabExperimentManager);
        labelExperimentManagerStatus->setObjectName("labelExperimentManagerStatus");
        labelExperimentManagerStatus->setWordWrap(true);

        verticalLayoutExperimentManager->addWidget(labelExperimentManagerStatus);

        labelExperimentManagerPlaceholder = new QLabel(tabExperimentManager);
        labelExperimentManagerPlaceholder->setObjectName("labelExperimentManagerPlaceholder");
        labelExperimentManagerPlaceholder->setWordWrap(true);

        verticalLayoutExperimentManager->addWidget(labelExperimentManagerPlaceholder);

        verticalSpacerExperimentManager = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayoutExperimentManager->addItem(verticalSpacerExperimentManager);

        tabWidgetSimulationConfigure->addTab(tabExperimentManager, QString());

        verticalLayout->addWidget(tabWidgetSimulationConfigure);

        buttonBox = new QDialogButtonBox(DialogSimulationConfigure);
        buttonBox->setObjectName("buttonBox");
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);

#if QT_CONFIG(shortcut)
        labelNumberOfReplications->setBuddy(spinBoxNumberOfReplications);
        labelReplicationBaseTimeUnit->setBuddy(comboBoxReplicationBaseTimeUnit);
        labelReplicationLength->setBuddy(doubleSpinBoxReplicationLength);
        labelWarmUpPeriod->setBuddy(doubleSpinBoxWarmUpPeriod);
        labelTerminatingCondition->setBuddy(plainTextTerminatingCondition);
        labelParallelizationThreads->setBuddy(spinBoxParallelizationThreads);
        labelParallelizationBatchSize->setBuddy(spinBoxParallelizationBatchSize);
        labelDistributedCoordinatorUrl->setBuddy(lineEditDistributedCoordinatorUrl);
        labelDistributedToken->setBuddy(lineEditDistributedToken);
#endif // QT_CONFIG(shortcut)
        QWidget::setTabOrder(tabWidgetSimulationConfigure, spinBoxNumberOfReplications);
        QWidget::setTabOrder(spinBoxNumberOfReplications, comboBoxReplicationBaseTimeUnit);
        QWidget::setTabOrder(comboBoxReplicationBaseTimeUnit, doubleSpinBoxReplicationLength);
        QWidget::setTabOrder(doubleSpinBoxReplicationLength, comboBoxReplicationLengthTimeUnit);
        QWidget::setTabOrder(comboBoxReplicationLengthTimeUnit, doubleSpinBoxWarmUpPeriod);
        QWidget::setTabOrder(doubleSpinBoxWarmUpPeriod, comboBoxWarmUpPeriodTimeUnit);
        QWidget::setTabOrder(comboBoxWarmUpPeriodTimeUnit, plainTextTerminatingCondition);
        QWidget::setTabOrder(plainTextTerminatingCondition, checkBoxInitializeSystem);
        QWidget::setTabOrder(checkBoxInitializeSystem, checkBoxInitializeStatistics);
        QWidget::setTabOrder(checkBoxInitializeStatistics, checkBoxStepByStep);
        QWidget::setTabOrder(checkBoxStepByStep, checkBoxPauseOnEvent);
        QWidget::setTabOrder(checkBoxPauseOnEvent, checkBoxPauseOnReplication);
        QWidget::setTabOrder(checkBoxPauseOnReplication, checkBoxParallelizationEnabled);
        QWidget::setTabOrder(checkBoxParallelizationEnabled, spinBoxParallelizationThreads);
        QWidget::setTabOrder(spinBoxParallelizationThreads, spinBoxParallelizationBatchSize);
        QWidget::setTabOrder(spinBoxParallelizationBatchSize, checkBoxDistributedParallelizationEnabled);
        QWidget::setTabOrder(checkBoxDistributedParallelizationEnabled, lineEditDistributedCoordinatorUrl);
        QWidget::setTabOrder(lineEditDistributedCoordinatorUrl, lineEditDistributedToken);
        QWidget::setTabOrder(lineEditDistributedToken, pushButtonConfigureDistributedParallelization);

        retranslateUi(DialogSimulationConfigure);
        QObject::connect(buttonBox, &QDialogButtonBox::accepted, DialogSimulationConfigure, qOverload<>(&QDialog::accept));
        QObject::connect(buttonBox, &QDialogButtonBox::rejected, DialogSimulationConfigure, qOverload<>(&QDialog::reject));

        tabWidgetSimulationConfigure->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(DialogSimulationConfigure);
    } // setupUi

    void retranslateUi(QDialog *DialogSimulationConfigure)
    {
        DialogSimulationConfigure->setWindowTitle(QCoreApplication::translate("DialogSimulationConfigure", "Configure Simulation", nullptr));
        labelNumberOfReplications->setText(QCoreApplication::translate("DialogSimulationConfigure", "Number of Replications:", nullptr));
        labelReplicationBaseTimeUnit->setText(QCoreApplication::translate("DialogSimulationConfigure", "Replication Base Time Unit:", nullptr));
        labelReplicationLength->setText(QCoreApplication::translate("DialogSimulationConfigure", "Replication Length:", nullptr));
        labelWarmUpPeriod->setText(QCoreApplication::translate("DialogSimulationConfigure", "Warmup Period:", nullptr));
        labelTerminatingCondition->setText(QCoreApplication::translate("DialogSimulationConfigure", "Terminating Condition:", nullptr));
        checkBoxInitializeSystem->setText(QCoreApplication::translate("DialogSimulationConfigure", "Initialize system", nullptr));
        checkBoxInitializeStatistics->setText(QCoreApplication::translate("DialogSimulationConfigure", "Initialize statistics between replications", nullptr));
        checkBoxStepByStep->setText(QCoreApplication::translate("DialogSimulationConfigure", "Step by step", nullptr));
        checkBoxPauseOnEvent->setText(QCoreApplication::translate("DialogSimulationConfigure", "Pause on event", nullptr));
        checkBoxPauseOnReplication->setText(QCoreApplication::translate("DialogSimulationConfigure", "Pause on replication", nullptr));
        tabWidgetSimulationConfigure->setTabText(tabWidgetSimulationConfigure->indexOf(tabModelSimulation), QCoreApplication::translate("DialogSimulationConfigure", "Model Simulation", nullptr));
        labelSimulationReporterStatus->setText(QCoreApplication::translate("DialogSimulationConfigure", "No SimulationReporter_if instance is loaded.", nullptr));
        labelSimulationReporterPlaceholder->setText(QCoreApplication::translate("DialogSimulationConfigure", "This tab is reserved for editable SimulationReporter_if settings when reporter implementations expose GUI-editable options.", nullptr));
        tabWidgetSimulationConfigure->setTabText(tabWidgetSimulationConfigure->indexOf(tabSimulationReport), QCoreApplication::translate("DialogSimulationConfigure", "Simulation Report", nullptr));
        labelParallelizationIntro->setText(QCoreApplication::translate("DialogSimulationConfigure", "Configure how simulation replications and experiment batches should be prepared for local or distributed parallel execution.", nullptr));
        groupBoxLocalParallelization->setTitle(QCoreApplication::translate("DialogSimulationConfigure", "Local parallelization", nullptr));
        checkBoxParallelizationEnabled->setText(QCoreApplication::translate("DialogSimulationConfigure", "Enable local parallel execution preparation", nullptr));
        labelParallelizationThreads->setText(QCoreApplication::translate("DialogSimulationConfigure", "Local Simulator instances / threads:", nullptr));
        labelParallelizationBatchSize->setText(QCoreApplication::translate("DialogSimulationConfigure", "Batch size per scheduling cycle:", nullptr));
        groupBoxDistributedParallelization->setTitle(QCoreApplication::translate("DialogSimulationConfigure", "Distributed parallelization", nullptr));
        checkBoxDistributedParallelizationEnabled->setText(QCoreApplication::translate("DialogSimulationConfigure", "Enable distributed execution preparation", nullptr));
        labelDistributedCoordinatorUrl->setText(QCoreApplication::translate("DialogSimulationConfigure", "Coordinator URL:", nullptr));
        lineEditDistributedCoordinatorUrl->setPlaceholderText(QCoreApplication::translate("DialogSimulationConfigure", "https://genesys.example.org/optimizer", nullptr));
        labelDistributedToken->setText(QCoreApplication::translate("DialogSimulationConfigure", "Access token:", nullptr));
        lineEditDistributedToken->setPlaceholderText(QCoreApplication::translate("DialogSimulationConfigure", "Optional token for a future Genesys web service", nullptr));
        pushButtonConfigureDistributedParallelization->setText(QCoreApplication::translate("DialogSimulationConfigure", "Configure Distributed Service...", nullptr));
        labelParallelizationStatus->setText(QCoreApplication::translate("DialogSimulationConfigure", "Local settings are stored in the GUI session. Distributed settings are a planned integration point for the Genesys web application.", nullptr));
        tabWidgetSimulationConfigure->setTabText(tabWidgetSimulationConfigure->indexOf(tabParallelization), QCoreApplication::translate("DialogSimulationConfigure", "Parallelization", nullptr));
        labelExperimentManagerStatus->setText(QCoreApplication::translate("DialogSimulationConfigure", "No ExperimentManager instance is loaded.", nullptr));
        labelExperimentManagerPlaceholder->setText(QCoreApplication::translate("DialogSimulationConfigure", "This tab is reserved for editable ExperimentManager settings as the experiment kernel API stabilizes.", nullptr));
        tabWidgetSimulationConfigure->setTabText(tabWidgetSimulationConfigure->indexOf(tabExperimentManager), QCoreApplication::translate("DialogSimulationConfigure", "Experiment Manager", nullptr));
    } // retranslateUi

};

namespace Ui {
    class DialogSimulationConfigure: public Ui_DialogSimulationConfigure {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOGSIMULATIONCONFIGURE_H
