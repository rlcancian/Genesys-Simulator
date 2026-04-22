#include "dialogsystempreferences.h"
#include "ui_dialogsystempreferences.h"

#include "../guithememanager.h"
#include "../systempreferences.h"
#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/TraceManager.h"

#include <QApplication>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QPushButton>

DialogSystemPreferences::DialogSystemPreferences(Simulator* simulator, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogSystemPreferences),
    _simulator(simulator) {
    ui->setupUi(this);
    populateTraceLevels();
    loadPreferencesIntoUi();

    connect(ui->pushButtonBrowseStartupModel, &QPushButton::clicked, this, &DialogSystemPreferences::browseSpecificModelFile);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, [this]() {
        savePreferencesFromUi();
        accept();
    });
}

DialogSystemPreferences::~DialogSystemPreferences() {
	delete ui;
}

void DialogSystemPreferences::populateTraceLevels() {
    ui->comboBoxTraceLevel->clear();
    ui->comboBoxTraceLevel->addItem(tr("L0 - no traces"), static_cast<int>(TraceManager::Level::L0_noTraces));
    ui->comboBoxTraceLevel->addItem(tr("L1 - fatal errors"), static_cast<int>(TraceManager::Level::L1_errorFatal));
    ui->comboBoxTraceLevel->addItem(tr("L2 - results"), static_cast<int>(TraceManager::Level::L2_results));
    ui->comboBoxTraceLevel->addItem(tr("L3 - recoverable errors/progress"), static_cast<int>(TraceManager::Level::L3_errorRecover));
    ui->comboBoxTraceLevel->addItem(tr("L4 - warnings"), static_cast<int>(TraceManager::Level::L4_warning));
    ui->comboBoxTraceLevel->addItem(tr("L5 - events"), static_cast<int>(TraceManager::Level::L5_event));
    ui->comboBoxTraceLevel->addItem(tr("L6 - arrivals"), static_cast<int>(TraceManager::Level::L6_arrival));
    ui->comboBoxTraceLevel->addItem(tr("L7 - internal"), static_cast<int>(TraceManager::Level::L7_internal));
    ui->comboBoxTraceLevel->addItem(tr("L8 - detailed"), static_cast<int>(TraceManager::Level::L8_detailed));
    ui->comboBoxTraceLevel->addItem(tr("L9 - most detailed"), static_cast<int>(TraceManager::Level::L9_mostDetailed));
}

void DialogSystemPreferences::loadPreferencesIntoUi() {
    ui->checkBoxStartMaximized->setChecked(SystemPreferences::startMaximized());
    ui->checkBoxAutoLoadPlugins->setChecked(SystemPreferences::autoLoadPlugins());
    ui->checkBoxCheckSystemPackagesAtStart->setChecked(SystemPreferences::checkSystemPackagesAtStart());
    ui->lineEditStartupModelFile->setText(QString::fromStdString(SystemPreferences::modelfilename()));
    ui->labelConfigPathValue->setText(SystemPreferences::configFilePath());

    switch (SystemPreferences::startupModelMode()) {
    case SystemPreferences::StartupModelMode::NoModel:
        ui->radioButtonStartNoModel->setChecked(true);
        break;
    case SystemPreferences::StartupModelMode::OpenSpecificModel:
        ui->radioButtonStartSpecificModel->setChecked(true);
        break;
    case SystemPreferences::StartupModelMode::OpenLastModel:
        ui->radioButtonStartLastModel->setChecked(true);
        break;
    case SystemPreferences::StartupModelMode::NewModel:
    default:
        ui->radioButtonStartNewModel->setChecked(true);
        break;
    }

    const int traceIndex = ui->comboBoxTraceLevel->findData(static_cast<int>(SystemPreferences::traceLevel()));
    ui->comboBoxTraceLevel->setCurrentIndex(traceIndex >= 0 ? traceIndex : ui->comboBoxTraceLevel->count() - 1);

    ui->comboBoxTheme->setCurrentIndex(SystemPreferences::visualTheme() == SystemPreferences::VisualTheme::Dark ? 1 : 0);
    ui->comboBoxInterfaceStyle->setCurrentIndex(SystemPreferences::interfaceStyle() == SystemPreferences::InterfaceStyle::Modern ? 1 : 0);
    ui->spinBoxFontPointSize->setValue(SystemPreferences::applicationFontPointSize());
    ui->checkBoxDiagramUsesThemeColors->setChecked(SystemPreferences::diagramUsesThemeColors());
    ui->spinBoxRecentModelsLimit->setValue(static_cast<int>(SystemPreferences::recentModelFilesLimit()));
    ui->comboBoxAutomaticPositioningStrategy->setCurrentIndex(
        SystemPreferences::automaticPositioningStrategy() == SystemPreferences::AutomaticPositioningStrategy::Legacy ? 1 : 0);
}

void DialogSystemPreferences::savePreferencesFromUi() {
    SystemPreferences::setStartMaximized(ui->checkBoxStartMaximized->isChecked());
    SystemPreferences::setAutoLoadPlugins(ui->checkBoxAutoLoadPlugins->isChecked());
    SystemPreferences::setCheckSystemPackagesAtStart(ui->checkBoxCheckSystemPackagesAtStart->isChecked());
    SystemPreferences::setModelfilename(ui->lineEditStartupModelFile->text().trimmed().toStdString());

    if (ui->radioButtonStartNoModel->isChecked()) {
        SystemPreferences::setStartupModelMode(SystemPreferences::StartupModelMode::NoModel);
    } else if (ui->radioButtonStartSpecificModel->isChecked()) {
        SystemPreferences::setStartupModelMode(SystemPreferences::StartupModelMode::OpenSpecificModel);
    } else if (ui->radioButtonStartLastModel->isChecked()) {
        SystemPreferences::setStartupModelMode(SystemPreferences::StartupModelMode::OpenLastModel);
    } else {
        SystemPreferences::setStartupModelMode(SystemPreferences::StartupModelMode::NewModel);
    }

    SystemPreferences::setTraceLevel(static_cast<TraceManager::Level>(ui->comboBoxTraceLevel->currentData().toInt()));
    if (_simulator != nullptr && _simulator->getTraceManager() != nullptr) {
        _simulator->getTraceManager()->setTraceLevel(SystemPreferences::traceLevel());
    }

    SystemPreferences::setVisualTheme(ui->comboBoxTheme->currentIndex() == 1
                                          ? SystemPreferences::VisualTheme::Dark
                                          : SystemPreferences::VisualTheme::Light);
    SystemPreferences::setInterfaceStyle(ui->comboBoxInterfaceStyle->currentIndex() == 1
                                             ? SystemPreferences::InterfaceStyle::Modern
                                             : SystemPreferences::InterfaceStyle::Classic);
    SystemPreferences::setApplicationFontPointSize(ui->spinBoxFontPointSize->value());
    SystemPreferences::setDiagramUsesThemeColors(ui->checkBoxDiagramUsesThemeColors->isChecked());
    SystemPreferences::setRecentModelFilesLimit(static_cast<unsigned int>(ui->spinBoxRecentModelsLimit->value()));
    SystemPreferences::setAutomaticPositioningStrategy(
        ui->comboBoxAutomaticPositioningStrategy->currentIndex() == 1
            ? SystemPreferences::AutomaticPositioningStrategy::Legacy
            : SystemPreferences::AutomaticPositioningStrategy::Centered);
    SystemPreferences::save();
    GuiThemeManager::applyApplicationTheme(qobject_cast<QApplication*>(QApplication::instance()));
}

void DialogSystemPreferences::browseSpecificModelFile() {
    const QString currentPath = ui->lineEditStartupModelFile->text().trimmed();
    const QString initialDirectory = currentPath.isEmpty()
                                         ? QString()
                                         : QFileInfo(currentPath).absolutePath();
    const QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Select startup model"),
        initialDirectory,
        tr("Genesys Model (*.gen);;Genesys Graphical User Interface (*.gui);;XML Files (*.xml);;JSON Files (*.json);;C++ Files (*.cpp)"),
        nullptr,
        QFileDialog::DontUseNativeDialog);
    if (!fileName.isEmpty()) {
        ui->lineEditStartupModelFile->setText(fileName);
        ui->radioButtonStartSpecificModel->setChecked(true);
    }
}
