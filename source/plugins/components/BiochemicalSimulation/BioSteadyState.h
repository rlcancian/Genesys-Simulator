/*
 * File:   BioSteadyState.h
 * Author: GenESyS
 *
 * Component that evaluates BioNetwork steady-state status.
 */

#ifndef BIOSTEADYSTATE_H
#define BIOSTEADYSTATE_H

#include <string>

#include "kernel/simulator/ModelComponent.h"
#include "kernel/simulator/Plugin.h"
#include "plugins/data/BiochemicalSimulation/BioNetwork.h"

/*!
 * \brief Component that checks steady-state diagnostics over BioNetwork results.
 *
 * BioSteadyState can optionally trigger one simulation run before evaluating
 * steady-state through BioNetwork analysis helpers.
 */
class BioSteadyState : public ModelComponent {
public:
	BioSteadyState(Model* model, std::string name = "");
	virtual ~BioSteadyState() override = default;

public:
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord* fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

public:
	void setBioNetwork(BioNetwork* bioNetwork);
	BioNetwork* getBioNetwork() const;
	void setTolerance(double tolerance);
	double getTolerance() const;
	void setRunSimulationBeforeCheck(bool runSimulationBeforeCheck);
	bool getRunSimulationBeforeCheck() const;
	void setLastSteady(bool lastSteady);
	bool getLastSteady() const;
	void setLastMaxAbsoluteDerivative(double lastMaxAbsoluteDerivative);
	double getLastMaxAbsoluteDerivative() const;
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
	void _forwardEntity(Entity* entity);

private:
	const struct DEFAULT_VALUES {
		std::string bioNetworkName = "";
		double tolerance = 1e-6;
		bool runSimulationBeforeCheck = false;
		bool lastSteady = false;
		double lastMaxAbsoluteDerivative = 0.0;
		std::string lastMessage = "";
	} DEFAULT;

	BioNetwork* _bioNetwork = nullptr;
	double _tolerance = DEFAULT.tolerance;
	bool _runSimulationBeforeCheck = DEFAULT.runSimulationBeforeCheck;
	bool _lastSteady = DEFAULT.lastSteady;
	double _lastMaxAbsoluteDerivative = DEFAULT.lastMaxAbsoluteDerivative;
	std::string _lastMessage = DEFAULT.lastMessage;
};

#endif /* BIOSTEADYSTATE_H */
