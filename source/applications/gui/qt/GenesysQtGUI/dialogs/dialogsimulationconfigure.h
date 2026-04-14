#ifndef DIALOGSIMULATIONCONFIGURE_H
#define DIALOGSIMULATIONCONFIGURE_H

#include "../../../../../kernel/simulator/ModelSimulation.h"

#include <QDialog>
#include <string>

class QComboBox;

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
	};

	void _populateTimeUnitComboBoxes();
	void _loadModelSimulation();
	SimulationConfiguration _configurationFromUi() const;
	bool _hasPendingChanges() const;
	Util::TimeUnit _timeUnitFromComboBox(const QComboBox* comboBox) const;
	void _setTimeUnitComboBoxIndex(QComboBox* comboBox, Util::TimeUnit timeUnit);
	void _applyIfChanged(const SimulationConfiguration& edited);

	Ui::DialogSimulationConfigure *ui;
	ModelSimulation* _modelSimulation = nullptr;
	SimulationConfiguration _originalConfiguration;
};

#endif // DIALOGSIMULATIONCONFIGURE_H
