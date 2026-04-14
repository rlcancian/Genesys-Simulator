#include "dialogsimulationconfigure.h"
#include "ui_dialogsimulationconfigure.h"

#include "../../../../../kernel/simulator/ExperimentManager.h"
#include "../../../../../kernel/simulator/SimulationReporter_if.h"

#include <QComboBox>
#include <QMessageBox>

DialogSimulationConfigure::DialogSimulationConfigure(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::DialogSimulationConfigure)
{
	ui->setupUi(this);
	_populateTimeUnitComboBoxes();
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

void DialogSimulationConfigure::accept()
{
	if (_modelSimulation != nullptr) {
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
	return configuration;
}

bool DialogSimulationConfigure::_hasPendingChanges() const
{
	if (_modelSimulation == nullptr) {
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
		|| edited.pauseOnReplication != _originalConfiguration.pauseOnReplication;
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
	if (edited.numberOfReplications != _originalConfiguration.numberOfReplications) {
		_modelSimulation->setNumberOfReplications(edited.numberOfReplications);
	}
	if (edited.replicationBaseTimeUnit != _originalConfiguration.replicationBaseTimeUnit) {
		_modelSimulation->setReplicationReportBaseTimeUnit(edited.replicationBaseTimeUnit);
	}
	if (edited.replicationLength != _originalConfiguration.replicationLength) {
		_modelSimulation->setReplicationLength(edited.replicationLength);
	}
	if (edited.replicationLengthTimeUnit != _originalConfiguration.replicationLengthTimeUnit) {
		_modelSimulation->setReplicationLengthTimeUnit(edited.replicationLengthTimeUnit);
	}
	if (edited.warmUpPeriod != _originalConfiguration.warmUpPeriod) {
		_modelSimulation->setWarmUpPeriod(edited.warmUpPeriod);
	}
	if (edited.warmUpPeriodTimeUnit != _originalConfiguration.warmUpPeriodTimeUnit) {
		_modelSimulation->setWarmUpPeriodTimeUnit(edited.warmUpPeriodTimeUnit);
	}
	if (edited.terminatingCondition != _originalConfiguration.terminatingCondition) {
		_modelSimulation->setTerminatingCondition(edited.terminatingCondition);
	}
	if (edited.initializeSystem != _originalConfiguration.initializeSystem) {
		_modelSimulation->setInitializeSystem(edited.initializeSystem);
	}
	if (edited.initializeStatistics != _originalConfiguration.initializeStatistics) {
		_modelSimulation->setInitializeStatistics(edited.initializeStatistics);
	}
	if (edited.stepByStep != _originalConfiguration.stepByStep) {
		_modelSimulation->setStepByStep(edited.stepByStep);
	}
	if (edited.pauseOnEvent != _originalConfiguration.pauseOnEvent) {
		_modelSimulation->setPauseOnEvent(edited.pauseOnEvent);
	}
	if (edited.pauseOnReplication != _originalConfiguration.pauseOnReplication) {
		_modelSimulation->setPauseOnReplication(edited.pauseOnReplication);
	}
}
