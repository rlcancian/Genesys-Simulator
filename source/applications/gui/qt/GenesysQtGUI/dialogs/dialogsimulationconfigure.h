#ifndef DIALOGSIMULATIONCONFIGURE_H
#define DIALOGSIMULATIONCONFIGURE_H

#include "kernel/simulator/ModelSimulation.h"

#include <QDialog>
#include <string>

class QComboBox;
class ExperimentManager;
class SimulationReporter_if;

namespace Ui {
	class DialogSimulationConfigure;
}

class DialogSimulationConfigure : public QDialog
{
	Q_OBJECT

public:
	explicit DialogSimulationConfigure(QWidget *parent = nullptr);
	~DialogSimulationConfigure();
	void setModelSimulation(ModelSimulation* modelSimulation);
	void setExperimentManager(ExperimentManager* experimentManager);
	void setParallelizationSettings(bool* enabled, int* localThreads, int* batchSize);
	/**
	 * @brief Binds the GUI-session flag that globally enables or disables simulation animations.
	 *
	 * This setting is intentionally kept in the GUI layer because it controls whether graphical
	 * animation work should be executed at runtime, without changing kernel simulation semantics.
	 */
	void setSimulationAnimationsEnabled(bool* enabled);

public slots:
	void accept() override;
	void reject() override;

private:
	struct SimulationConfiguration {
		unsigned int numberOfReplications = 1;
		Util::TimeUnit replicationBaseTimeUnit = Util::TimeUnit::unknown;
		double replicationLength = 0.0;
		Util::TimeUnit replicationLengthTimeUnit = Util::TimeUnit::unknown;
		double warmUpPeriod = 0.0;
		Util::TimeUnit warmUpPeriodTimeUnit = Util::TimeUnit::unknown;
		std::string terminatingCondition;
		bool initializeSystem = true;
		bool initializeStatistics = true;
		bool stepByStep = false;
		bool pauseOnEvent = false;
		bool pauseOnReplication = false;
		bool parallelizationEnabled = false;
		int parallelizationThreads = 1;
		int parallelizationBatchSize = 100;
		bool distributedParallelizationEnabled = false;
		std::string distributedCoordinatorUrl;
		std::string distributedToken;
		bool simulationAnimationsEnabled = true;
	};

	void _populateTimeUnitComboBoxes();
	void _loadModelSimulation();
	void _loadSimulationReporter();
	void _loadExperimentManager();
	void _loadParallelizationSettings();
	/** @brief Loads the current GUI-session animation toggle into the dialog widgets. */
	void _loadSimulationAnimationSettings();
	SimulationConfiguration _configurationFromUi() const;
	bool _hasPendingChanges() const;
	Util::TimeUnit _timeUnitFromComboBox(const QComboBox* comboBox) const;
	void _setTimeUnitComboBoxIndex(QComboBox* comboBox, Util::TimeUnit timeUnit);
	void _applyIfChanged(const SimulationConfiguration& edited);

	Ui::DialogSimulationConfigure *ui;
	ModelSimulation* _modelSimulation = nullptr;
	SimulationReporter_if* _simulationReporter = nullptr;
	ExperimentManager* _experimentManager = nullptr;
	bool* _parallelizationEnabled = nullptr;
	int* _parallelizationThreads = nullptr;
	int* _parallelizationBatchSize = nullptr;
	bool* _simulationAnimationsEnabled = nullptr;
	SimulationConfiguration _originalConfiguration;
};

#endif // DIALOGSIMULATIONCONFIGURE_H
