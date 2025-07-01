#include "dialogsimulationconfigure.h"
#include "ui_dialogsimulationconfigure.h"

DialogSimulationConfigure::DialogSimulationConfigure(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogSimulationConfigure)
{
    ui->setupUi(this);
}

DialogSimulationConfigure::~DialogSimulationConfigure()
{
    delete ui;
}

// Set the attributes required to save in model
void DialogSimulationConfigure::setSimulator(Simulator * sim) {
    ms = sim->getModelManager()->current()->getSimulation();
    trace = sim->getTraceManager();
    infos = sim->getModelManager()->current()->getInfos();

}

// Exeperiment name
void DialogSimulationConfigure::on_plainTextEdit_textChanged()
{
    experimentName = ui->plainTextEdit->toPlainText().toStdString();
}

// Number of replication
void DialogSimulationConfigure::on_spinBox_valueChanged(int arg1)
{
    numberOfReplication = arg1;
}

// Replication Length
void DialogSimulationConfigure::on_spinBox_3_textChanged(const QString &arg1)
{
    replicationLength = arg1.toDouble();
}

// Replication Length time unit
void DialogSimulationConfigure::on_comboBox_currentIndexChanged(int index)
{
    switch (index) {
    case 0:
        replicationLengthtimeUnit = Util::TimeUnit::unknown;
        break;
    case 1:
        replicationLengthtimeUnit = Util::TimeUnit::picosecond;
        break;
    case 2:
        replicationLengthtimeUnit = Util::TimeUnit::nanosecond;
        break;
    case 3:
        replicationLengthtimeUnit = Util::TimeUnit::microsecond;
        break;
    case 4:
        replicationLengthtimeUnit = Util::TimeUnit::milisecond;
        break;
    case 5:
        replicationLengthtimeUnit = Util::TimeUnit::second;
        break;
    case 6:
        replicationLengthtimeUnit = Util::TimeUnit::minute;
        break;
    case 7:
        replicationLengthtimeUnit = Util::TimeUnit::hour;
        break;
    case 8:
        replicationLengthtimeUnit = Util::TimeUnit::day;
        break;
    case 9:
        replicationLengthtimeUnit = Util::TimeUnit::week;
        break;
    default:
        replicationLengthtimeUnit = Util::TimeUnit::unknown;
        break;
    }
}

// Warm up Period
void DialogSimulationConfigure::on_spinBox_2_textChanged(const QString &arg1)
{
    warmUpPeriod = arg1.toDouble();
}

// Warm up Period time unit
void DialogSimulationConfigure::on_comboBox_2_currentIndexChanged(int index)
{
    switch (index) {
    case 0:
        warmUpPeriodTimeUnit = Util::TimeUnit::unknown;
        break;
    case 1:
        warmUpPeriodTimeUnit = Util::TimeUnit::picosecond;
        break;
    case 2:
        warmUpPeriodTimeUnit = Util::TimeUnit::nanosecond;
        break;
    case 3:
        warmUpPeriodTimeUnit = Util::TimeUnit::microsecond;
        break;
    case 4:
        warmUpPeriodTimeUnit = Util::TimeUnit::milisecond;
        break;
    case 5:
        warmUpPeriodTimeUnit = Util::TimeUnit::second;
        break;
    case 6:
        warmUpPeriodTimeUnit = Util::TimeUnit::minute;
        break;
    case 7:
        warmUpPeriodTimeUnit = Util::TimeUnit::hour;
        break;
    case 8:
        warmUpPeriodTimeUnit = Util::TimeUnit::day;
        break;
    case 9:
        warmUpPeriodTimeUnit = Util::TimeUnit::week;
        break;
    }
}

// Terminating Codition
void DialogSimulationConfigure::on_plainTextEdit_2_textChanged()
{
   terminatingCondition = ui->plainTextEdit_2->toPlainText().toStdString();
}

// Trace Level
void DialogSimulationConfigure::on_comboBox_3_currentIndexChanged(int index)
{
   switch (index) {
   case 0:
        traceLevel = TraceManager::Level::L0_noTraces;
        break;
   case 1:
        traceLevel = TraceManager::Level::L1_errorFatal;
        break;
   case 2:
        traceLevel = TraceManager::Level::L2_results;
        break;
   case 3:
        traceLevel = TraceManager::Level::L3_errorRecover;
        break;
   case 4:
        traceLevel = TraceManager::Level::L4_warning;
        break;
   case 5:
        traceLevel = TraceManager::Level::L5_event;
        break;
   case 6:
        traceLevel = TraceManager::Level::L6_arrival;
        break;
   case 7:
        traceLevel = TraceManager::Level::L7_internal;
        break;
   case 8:
        traceLevel = TraceManager::Level::L8_detailed;
        break;
   case 9:
        traceLevel = TraceManager::Level::L9_mostDetailed;
        break;
   default:
        traceLevel = TraceManager::Level::L0_noTraces;
        break;
   }
}

// InitializeSystem
void DialogSimulationConfigure::on_checkBox_stateChanged(int arg1)
{
   initializeSystem = arg1;
}

// initialize Statistics
void DialogSimulationConfigure::on_checkBox_4_stateChanged(int arg1)
{
   initializeStatistics = arg1;
}

// Show Reports after Replication
void DialogSimulationConfigure::on_checkBox_3_stateChanged(int arg1)
{
   showReportsAfterReplication = arg1;
}

// Show Reports after Simulation
void DialogSimulationConfigure::on_checkBox_2_stateChanged(int arg1)
{
   showReportsAfterSimulation = arg1;
}

// Ok Button
void DialogSimulationConfigure::on_buttonBox_accepted()
{
   infos->setName(experimentName);

   ms->setNumberOfReplications(numberOfReplication);

   ms->setReplicationLength(replicationLength);
   ms->setReplicationLengthTimeUnit(replicationLengthtimeUnit);

   ms->setWarmUpPeriod(warmUpPeriod);
   ms->setWarmUpPeriodTimeUnit(warmUpPeriodTimeUnit);

   ms->setTerminatingCondition(terminatingCondition);
   trace->setTraceLevel(traceLevel);
   ms->setInitializeSystem(initializeSystem);
   ms->setInitializeStatistics(initializeStatistics);
   ms->setShowReportsAfterReplication(showReportsAfterReplication);
   ms->setShowReportsAfterSimulation(showReportsAfterSimulation);
}

// Set previous configuration when it's cancel
void DialogSimulationConfigure::on_buttonBox_rejected()
{
   previousConfiguration();
}
// Get previous configuration from model and update ui
void DialogSimulationConfigure::previousConfiguration() {

   // Missing name
   experimentName = infos->getName();
   numberOfReplication = ms->getNumberOfReplications();
   replicationLength = ms->getReplicationLength();
   replicationLengthtimeUnit = ms->getReplicationLengthTimeUnit();
   warmUpPeriod = ms->getWarmUpPeriod();
   warmUpPeriodTimeUnit = ms->getWarmUpPeriodTimeUnit();
   terminatingCondition =  ms->getTerminatingCondition();
   traceLevel = trace->getTraceLevel();

   initializeSystem =   ms->isInitializeSystem();
   initializeStatistics = ms->isInitializeStatistics();
   showReportsAfterReplication =  ms->isShowReportsAfterReplication();
   showReportsAfterSimulation = ms->isShowReportsAfterSimulation();

   ui->plainTextEdit->setPlainText(QString::fromStdString(experimentName));
   ui->spinBox->setValue(numberOfReplication);
   ui->spinBox_3->setValue(replicationLength);
   ui->comboBox->setCurrentIndex(static_cast<int>(replicationLengthtimeUnit));
   ui->spinBox_2->setValue(warmUpPeriod);
   ui->comboBox_2->setCurrentIndex(static_cast<int>(warmUpPeriodTimeUnit));
   ui->plainTextEdit_2->setPlainText(QString::fromStdString(terminatingCondition));
   ui->comboBox_3->setCurrentIndex(static_cast<int>(traceLevel));

   ui->checkBox->setChecked(initializeSystem);
   ui->checkBox_2->setChecked(showReportsAfterSimulation);
   ui->checkBox_3->setChecked(showReportsAfterReplication);
   ui->checkBox_4->setChecked(initializeStatistics);
}

