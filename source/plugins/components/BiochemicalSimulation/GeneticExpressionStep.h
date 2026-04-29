#ifndef GENETICEXPRESSIONSTEP_H
#define GENETICEXPRESSIONSTEP_H

#include <string>

#include "kernel/simulator/ModelComponent.h"
#include "kernel/simulator/Plugin.h"
#include "plugins/data/BiochemicalSimulation/GeneticCircuit.h"

class GeneticCircuitPart;

/**
 * Component that executes one expression update step over a GeneticCircuit.
 */
class GeneticExpressionStep : public ModelComponent {
public:
	GeneticExpressionStep(Model* model, std::string name = "");
	virtual ~GeneticExpressionStep() override = default;

public:
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord* fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

public:
	void setGeneticCircuit(GeneticCircuit* geneticCircuit);
	GeneticCircuit* getGeneticCircuit() const;
	void setTimeStep(double timeStep);
	double getTimeStep() const;
	void setApplyRegulation(bool applyRegulation);
	bool getApplyRegulation() const;
	void setLastSucceeded(bool lastSucceeded);
	bool getLastSucceeded() const;
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
	void _forwardEntity(Entity* entity);

private:
	const struct DEFAULT_VALUES {
		std::string geneticCircuitName = "";
		double timeStep = 1.0;
		bool applyRegulation = true;
		bool lastSucceeded = false;
		double lastTotalExpression = 0.0;
		std::string lastMessage = "";
	} DEFAULT;

	GeneticCircuit* _geneticCircuit = nullptr;
	double _timeStep = DEFAULT.timeStep;
	bool _applyRegulation = DEFAULT.applyRegulation;
	bool _lastSucceeded = DEFAULT.lastSucceeded;
	double _lastTotalExpression = DEFAULT.lastTotalExpression;
	std::string _lastMessage = DEFAULT.lastMessage;
};

#endif /* GENETICEXPRESSIONSTEP_H */
