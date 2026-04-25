/*
 * File:   BioSimulate.h
 * Author: GenESyS
 *
 * Component that executes BioNetwork simulation when entities arrive.
 */

#ifndef BIOSIMULATE_H
#define BIOSIMULATE_H

#include <string>

#include "kernel/simulator/ModelComponent.h"
#include "kernel/simulator/Plugin.h"
#include "plugins/data/BiochemicalSimulation/BioNetwork.h"

/*!
 * \brief Component that triggers deterministic BioNetwork simulation.
 *
 * BioSimulate delegates simulation execution to a referenced BioNetwork. Each
 * arriving entity can either run the network with its own configured time
 * window or with component-provided start/stop/step values.
 */
class BioSimulate : public ModelComponent {
public:
	BioSimulate(Model* model, std::string name = "");
	virtual ~BioSimulate() override = default;

public:
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord* fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

public:
	void setBioNetwork(BioNetwork* bioNetwork);
	BioNetwork* getBioNetwork() const;
	void setUseNetworkTimeWindow(bool useNetworkTimeWindow);
	bool getUseNetworkTimeWindow() const;
	void setStartTime(double startTime);
	double getStartTime() const;
	void setStopTime(double stopTime);
	double getStopTime() const;
	void setStepSize(double stepSize);
	double getStepSize() const;
	void setLastSucceeded(bool lastSucceeded);
	bool getLastSucceeded() const;
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
		bool useNetworkTimeWindow = true;
		double startTime = 0.0;
		double stopTime = 1.0;
		double stepSize = 0.1;
		bool lastSucceeded = false;
		std::string lastMessage = "";
	} DEFAULT;

	BioNetwork* _bioNetwork = nullptr;
	bool _useNetworkTimeWindow = DEFAULT.useNetworkTimeWindow;
	double _startTime = DEFAULT.startTime;
	double _stopTime = DEFAULT.stopTime;
	double _stepSize = DEFAULT.stepSize;
	bool _lastSucceeded = DEFAULT.lastSucceeded;
	std::string _lastMessage = DEFAULT.lastMessage;
};

#endif /* BIOSIMULATE_H */
