#include "dialogsimulationconfigure.h"
#include "ui_dialogsimulationconfigure.h"

#include "../../../../../kernel/simulator/ExperimentManager.h"
#include "../../../../../kernel/simulator/SimulationReporter_if.h"

#include <QComboBox>
#include <QMessageBox>
#include <QPushButton>
#include <algorithm>

DialogSimulationConfigure::DialogSimulationConfigure(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::DialogSimulationConfigure)
{
	ui->setupUi(this);
	_populateTimeUnitComboBoxes();
	connect(ui->pushButtonConfigureDistributedParallelization, &QPushButton::clicked, this, [this]() {
		QMessageBox::information(
			this,
			tr("Distributed Parallelization"),
			tr("This button is a placeholder for a future Genesys distributed execution service or web application. "
			   "For now, local parallelization settings can be saved in this dialog."));
	});
}

DialogSimulationConfigure::~DialogSimulationConfigure()
{
	delete ui;
}

void DialogSimulationConfigure::setModelSimulation(ModelSimulation* modelSimulation)
{
	_modelSimulation = modelSimulation;
	_simulationReporter = _modelSimulation != nullptr ? _modelSimulation->getReporter() : nullptr;
	_loadModelSimulation();
	_loadSimulationReporter();
}

void DialogSimulationConfigure::setExperimentManager(ExperimentManager* experimentManager)
{
	_experimentManager = experimentManager;
	_loadExperimentManager();
}

void DialogSimulationConfigure::setParallelizationSettings(bool* enabled, int* localThreads, int* batchSize)
{
	_parallelizationEnabled = enabled;
	_parallelizationThreads = localThreads;
	_parallelizationBatchSize = batchSize;
	_loadParallelizationSettings();
}

void DialogSimulationConfigure::accept()
{
	if (_modelSimulation != nullptr || _parallelizationEnabled != nullptr || _parallelizationThreads != nullptr || _parallelizationBatchSize != nullptr) {
		_applyIfChanged(_configurationFromUi());
	}

	QDialog::accept();
}

void DialogSimulationConfigure::reject()
{
	if (_hasPendingChanges()) {
		const QMessageBox::StandardButton answer = QMessageBox::question(
			this,
			tr("Discard changes?"),
			tr("The simulation configuration has unsaved changes. Do you want to discard them?"),
			QMessageBox::Discard | QMessageBox::Cancel,
			QMessageBox::Cancel
		);
		if (answer != QMessageBox::Discard) {
			return;
		}
	}

	QDialog::reject();
}

void DialogSimulationConfigure::_populateTimeUnitComboBoxes()
{
	const QList<QComboBox*> comboBoxes = {
		ui->comboBoxReplicationBaseTimeUnit,
		ui->comboBoxReplicationLengthTimeUnit,
		ui->comboBoxWarmUpPeriodTimeUnit
	};

	for (QComboBox* comboBox : comboBoxes) {
		comboBox->addItem(tr("unknown"), static_cast<int>(Util::TimeUnit::unknown));
		comboBox->addItem(tr("picosecond"), static_cast<int>(Util::TimeUnit::picosecond));
		comboBox->addItem(tr("nanosecond"), static_cast<int>(Util::TimeUnit::nanosecond));
		comboBox->addItem(tr("microsecond"), static_cast<int>(Util::TimeUnit::microsecond));
		comboBox->addItem(tr("milisecond"), static_cast<int>(Util::TimeUnit::milisecond));
		comboBox->addItem(tr("second"), static_cast<int>(Util::TimeUnit::second));
		comboBox->addItem(tr("minute"), static_cast<int>(Util::TimeUnit::minute));
		comboBox->addItem(tr("hour"), static_cast<int>(Util::TimeUnit::hour));
		comboBox->addItem(tr("day"), static_cast<int>(Util::TimeUnit::day));
		comboBox->addItem(tr("week"), static_cast<int>(Util::TimeUnit::week));
	}
}

void DialogSimulationConfigure::_loadModelSimulation()
{
	if (_modelSimulation == nullptr) {
		return;
	}

	_originalConfiguration.numberOfReplications = _modelSimulation->getNumberOfReplications();
	_originalConfiguration.replicationBaseTimeUnit = _modelSimulation->getReplicationBaseTimeUnit();
	_originalConfiguration.replicationLength = _modelSimulation->getReplicationLength();
	_originalConfiguration.replicationLengthTimeUnit = _modelSimulation->getReplicationLengthTimeUnit();
	_originalConfiguration.warmUpPeriod = _modelSimulation->getWarmUpPeriod();
	_originalConfiguration.warmUpPeriodTimeUnit = _modelSimulation->getWarmUpPeriodTimeUnit();
	_originalConfiguration.terminatingCondition = _modelSimulation->getTerminatingCondition();
	_originalConfiguration.initializeSystem = _modelSimulation->isInitializeSystem();
	_originalConfiguration.initializeStatistics = _modelSimulation->isInitializeStatistics();
	_originalConfiguration.stepByStep = _modelSimulation->isStepByStep();
	_originalConfiguration.pauseOnEvent = _modelSimulation->isPauseOnEvent();
	_originalConfiguration.pauseOnReplication = _modelSimulation->isPauseOnReplication();

	// Keep the widgets as a direct view of the editable ModelSimulation attributes.
	ui->spinBoxNumberOfReplications->setValue(static_cast<int>(_originalConfiguration.numberOfReplications));
	_setTimeUnitComboBoxIndex(ui->comboBoxReplicationBaseTimeUnit, _originalConfiguration.replicationBaseTimeUnit);
	ui->doubleSpinBoxReplicationLength->setValue(_originalConfiguration.replicationLength);
	_setTimeUnitComboBoxIndex(ui->comboBoxReplicationLengthTimeUnit, _originalConfiguration.replicationLengthTimeUnit);
	ui->doubleSpinBoxWarmUpPeriod->setValue(_originalConfiguration.warmUpPeriod);
	_setTimeUnitComboBoxIndex(ui->comboBoxWarmUpPeriodTimeUnit, _originalConfiguration.warmUpPeriodTimeUnit);
	ui->plainTextTerminatingCondition->setPlainText(QString::fromStdString(_originalConfiguration.terminatingCondition));
	ui->checkBoxInitializeSystem->setChecked(_originalConfiguration.initializeSystem);
	ui->checkBoxInitializeStatistics->setChecked(_originalConfiguration.initializeStatistics);
	ui->checkBoxStepByStep->setChecked(_originalConfiguration.stepByStep);
	ui->checkBoxPauseOnEvent->setChecked(_originalConfiguration.pauseOnEvent);
	ui->checkBoxPauseOnReplication->setChecked(_originalConfiguration.pauseOnReplication);
}

void DialogSimulationConfigure::_loadSimulationReporter()
{
	if (_simulationReporter == nullptr) {
		ui->labelSimulationReporterStatus->setText(tr("No SimulationReporter_if instance is loaded."));
		return;
	}

	// The reporter interface currently exposes report actions, but no editable configuration properties.
	ui->labelSimulationReporterStatus->setText(tr("SimulationReporter_if instance is loaded for the current ModelSimulation."));
}

void DialogSimulationConfigure::_loadExperimentManager()
{
	if (_experimentManager == nullptr) {
		ui->labelExperimentManagerStatus->setText(tr("No ExperimentManager instance is loaded."));
		return;
	}

	// The current kernel exposes the concrete ExperimentManager while the ExperimentManager_if API evolves.
	ui->labelExperimentManagerStatus->setText(
		tr("ExperimentManager instance is loaded. Experiments: %1.").arg(_experimentManager->size())
	);
}

void DialogSimulationConfigure::_loadParallelizationSettings()
{
	_originalConfiguration.parallelizationEnabled = _parallelizationEnabled != nullptr ? *_parallelizationEnabled : false;
	_originalConfiguration.parallelizationThreads = _parallelizationThreads != nullptr ? *_parallelizationThreads : 1;
	_originalConfiguration.parallelizationBatchSize = _parallelizationBatchSize != nullptr ? *_parallelizationBatchSize : 100;
	_originalConfiguration.distributedParallelizationEnabled = false;
	_originalConfiguration.distributedCoordinatorUrl.clear();
	_originalConfiguration.distributedToken.clear();

	ui->checkBoxParallelizationEnabled->setChecked(_originalConfiguration.parallelizationEnabled);
	ui->spinBoxParallelizationThreads->setValue(std::max(1, _originalConfiguration.parallelizationThreads));
	ui->spinBoxParallelizationBatchSize->setValue(std::max(1, _originalConfiguration.parallelizationBatchSize));
	ui->checkBoxDistributedParallelizationEnabled->setChecked(_originalConfiguration.distributedParallelizationEnabled);
	ui->lineEditDistributedCoordinatorUrl->setText(QString::fromStdString(_originalConfiguration.distributedCoordinatorUrl));
	ui->lineEditDistributedToken->setText(QString::fromStdString(_originalConfiguration.distributedToken));
}

DialogSimulationConfigure::SimulationConfiguration DialogSimulationConfigure::_configurationFromUi() const
{
	SimulationConfiguration configuration;
	configuration.numberOfReplications = static_cast<unsigned int>(ui->spinBoxNumberOfReplications->value());
	configuration.replicationBaseTimeUnit = _timeUnitFromComboBox(ui->comboBoxReplicationBaseTimeUnit);
	configuration.replicationLength = ui->doubleSpinBoxReplicationLength->value();
	configuration.replicationLengthTimeUnit = _timeUnitFromComboBox(ui->comboBoxReplicationLengthTimeUnit);
	configuration.warmUpPeriod = ui->doubleSpinBoxWarmUpPeriod->value();
	configuration.warmUpPeriodTimeUnit = _timeUnitFromComboBox(ui->comboBoxWarmUpPeriodTimeUnit);
	configuration.terminatingCondition = ui->plainTextTerminatingCondition->toPlainText().toStdString();
	configuration.initializeSystem = ui->checkBoxInitializeSystem->isChecked();
	configuration.initializeStatistics = ui->checkBoxInitializeStatistics->isChecked();
	configuration.stepByStep = ui->checkBoxStepByStep->isChecked();
	configuration.pauseOnEvent = ui->checkBoxPauseOnEvent->isChecked();
	configuration.pauseOnReplication = ui->checkBoxPauseOnReplication->isChecked();
	configuration.parallelizationEnabled = ui->checkBoxParallelizationEnabled->isChecked();
	configuration.parallelizationThreads = ui->spinBoxParallelizationThreads->value();
	configuration.parallelizationBatchSize = ui->spinBoxParallelizationBatchSize->value();
	configuration.distributedParallelizationEnabled = ui->checkBoxDistributedParallelizationEnabled->isChecked();
	configuration.distributedCoordinatorUrl = ui->lineEditDistributedCoordinatorUrl->text().toStdString();
	configuration.distributedToken = ui->lineEditDistributedToken->text().toStdString();
	return configuration;
}

bool DialogSimulationConfigure::_hasPendingChanges() const
{
	if (_modelSimulation == nullptr && _parallelizationEnabled == nullptr && _parallelizationThreads == nullptr && _parallelizationBatchSize == nullptr) {
		return false;
	}

	const SimulationConfiguration edited = _configurationFromUi();
	return edited.numberOfReplications != _originalConfiguration.numberOfReplications
		|| edited.replicationBaseTimeUnit != _originalConfiguration.replicationBaseTimeUnit
		|| edited.replicationLength != _originalConfiguration.replicationLength
		|| edited.replicationLengthTimeUnit != _originalConfiguration.replicationLengthTimeUnit
		|| edited.warmUpPeriod != _originalConfiguration.warmUpPeriod
		|| edited.warmUpPeriodTimeUnit != _originalConfiguration.warmUpPeriodTimeUnit
		|| edited.terminatingCondition != _originalConfiguration.terminatingCondition
		|| edited.initializeSystem != _originalConfiguration.initializeSystem
		|| edited.initializeStatistics != _originalConfiguration.initializeStatistics
		|| edited.stepByStep != _originalConfiguration.stepByStep
		|| edited.pauseOnEvent != _originalConfiguration.pauseOnEvent
		|| edited.pauseOnReplication != _originalConfiguration.pauseOnReplication
		|| edited.parallelizationEnabled != _originalConfiguration.parallelizationEnabled
		|| edited.parallelizationThreads != _originalConfiguration.parallelizationThreads
		|| edited.parallelizationBatchSize != _originalConfiguration.parallelizationBatchSize
		|| edited.distributedParallelizationEnabled != _originalConfiguration.distributedParallelizationEnabled
		|| edited.distributedCoordinatorUrl != _originalConfiguration.distributedCoordinatorUrl
		|| edited.distributedToken != _originalConfiguration.distributedToken;
}

Util::TimeUnit DialogSimulationConfigure::_timeUnitFromComboBox(const QComboBox* comboBox) const
{
	const QVariant value = comboBox->currentData();
	return value.isValid() ? static_cast<Util::TimeUnit>(value.toInt()) : Util::TimeUnit::unknown;
}

void DialogSimulationConfigure::_setTimeUnitComboBoxIndex(QComboBox* comboBox, Util::TimeUnit timeUnit)
{
	const int index = comboBox->findData(static_cast<int>(timeUnit));
	comboBox->setCurrentIndex(index >= 0 ? index : 0);
}

void DialogSimulationConfigure::_applyIfChanged(const SimulationConfiguration& edited)
{
	if (_modelSimulation != nullptr && edited.numberOfReplications != _originalConfiguration.numberOfReplications) {
		_modelSimulation->setNumberOfReplications(edited.numberOfReplications);
	}
	if (_modelSimulation != nullptr && edited.replicationBaseTimeUnit != _originalConfiguration.replicationBaseTimeUnit) {
		_modelSimulation->setReplicationReportBaseTimeUnit(edited.replicationBaseTimeUnit);
	}
	if (_modelSimulation != nullptr && edited.replicationLength != _originalConfiguration.replicationLength) {
		_modelSimulation->setReplicationLength(edited.replicationLength);
	}
	if (_modelSimulation != nullptr && edited.replicationLengthTimeUnit != _originalConfiguration.replicationLengthTimeUnit) {
		_modelSimulation->setReplicationLengthTimeUnit(edited.replicationLengthTimeUnit);
	}
	if (_modelSimulation != nullptr && edited.warmUpPeriod != _originalConfiguration.warmUpPeriod) {
		_modelSimulation->setWarmUpPeriod(edited.warmUpPeriod);
	}
	if (_modelSimulation != nullptr && edited.warmUpPeriodTimeUnit != _originalConfiguration.warmUpPeriodTimeUnit) {
		_modelSimulation->setWarmUpPeriodTimeUnit(edited.warmUpPeriodTimeUnit);
	}
	if (_modelSimulation != nullptr && edited.terminatingCondition != _originalConfiguration.terminatingCondition) {
		_modelSimulation->setTerminatingCondition(edited.terminatingCondition);
	}
	if (_modelSimulation != nullptr && edited.initializeSystem != _originalConfiguration.initializeSystem) {
		_modelSimulation->setInitializeSystem(edited.initializeSystem);
	}
	if (_modelSimulation != nullptr && edited.initializeStatistics != _originalConfiguration.initializeStatistics) {
		_modelSimulation->setInitializeStatistics(edited.initializeStatistics);
	}
	if (_modelSimulation != nullptr && edited.stepByStep != _originalConfiguration.stepByStep) {
		_modelSimulation->setStepByStep(edited.stepByStep);
	}
	if (_modelSimulation != nullptr && edited.pauseOnEvent != _originalConfiguration.pauseOnEvent) {
		_modelSimulation->setPauseOnEvent(edited.pauseOnEvent);
	}
	if (_modelSimulation != nullptr && edited.pauseOnReplication != _originalConfiguration.pauseOnReplication) {
		_modelSimulation->setPauseOnReplication(edited.pauseOnReplication);
	}
	if (_parallelizationEnabled != nullptr) {
		*_parallelizationEnabled = edited.parallelizationEnabled;
	}
	if (_parallelizationThreads != nullptr) {
		*_parallelizationThreads = edited.parallelizationThreads;
	}
	if (_parallelizationBatchSize != nullptr) {
		*_parallelizationBatchSize = edited.parallelizationBatchSize;
	}
	_originalConfiguration = edited;
}
