#ifndef GENETICCIRCUITSIMULATE_H
#define GENETICCIRCUITSIMULATE_H

#include <string>

#include "kernel/simulator/ModelComponent.h"
#include "kernel/simulator/Plugin.h"
#include "plugins/data/BiochemicalSimulation/GeneticCircuit.h"

class GeneticCircuitPart;

/**
 * Component that simulates a GeneticCircuit over a time window.
 */
class GeneticCircuitSimulate : public ModelComponent {
public:
	GeneticCircuitSimulate(Model* model, std::string name = "");
	virtual ~GeneticCircuitSimulate() override = default;

public:
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord* fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

public:
	void setGeneticCircuit(GeneticCircuit* geneticCircuit);
	GeneticCircuit* getGeneticCircuit() const;
	void setStartTime(double startTime);
	double getStartTime() const;
	void setStopTime(double stopTime);
	double getStopTime() const;
	void setStepSize(double stepSize);
	double getStepSize() const;
	void setApplyRegulation(bool applyRegulation);
	bool getApplyRegulation() const;
	void setLastSucceeded(bool lastSucceeded);
	bool getLastSucceeded() const;
	void setLastSampleCount(unsigned int lastSampleCount);
	unsigned int getLastSampleCount() const;
	void setLastTotalExpression(double lastTotalExpression);
	double getLastTotalExpression() const;
	void setLastMessage(std::string lastMessage);
	std::string getLastMessage() const;

	virtual std::string show() override;

protected:
	virtual bool _loadInstance(PersistenceRecord* fields) override;
	virtual void _saveInstance(PersistenceRecord* fields, bool saveDefaultValues) override;
	virtual bool _check(std::string& errorMessage) override;
	virtual void _createInternalAndAttachedData() override;
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override;

protected:
	void _doCreateReportStatisticsDataDefinitions();
	void _doCreateEditableDataDefinitions();
	void _doCreateOthersDataDefinitions();

	void _createReportStatisticsDataDefinitions() override;
	void _createEditableDataDefinitions() override;
	void _createOthersDataDefinitions() override;

private:
	double _computeRegulationMultiplier(const GeneticCircuitPart* part) const;
	void _executeStep(double timeStep);
	void _forwardEntity(Entity* entity);

private:
	const struct DEFAULT_VALUES {
		std::string geneticCircuitName = "";
		double startTime = 0.0;
		double stopTime = 10.0;
		double stepSize = 1.0;
		bool applyRegulation = true;
		bool lastSucceeded = false;
		unsigned int lastSampleCount = 0;
		double lastTotalExpression = 0.0;
		std::string lastMessage = "";
	} DEFAULT;

	GeneticCircuit* _geneticCircuit = nullptr;
	double _startTime = DEFAULT.startTime;
	double _stopTime = DEFAULT.stopTime;
	double _stepSize = DEFAULT.stepSize;
	bool _applyRegulation = DEFAULT.applyRegulation;
	bool _lastSucceeded = DEFAULT.lastSucceeded;
	unsigned int _lastSampleCount = DEFAULT.lastSampleCount;
	double _lastTotalExpression = DEFAULT.lastTotalExpression;
	std::string _lastMessage = DEFAULT.lastMessage;
};

#endif /* GENETICCIRCUITSIMULATE_H */
