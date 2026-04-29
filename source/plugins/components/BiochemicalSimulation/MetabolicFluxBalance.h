#ifndef METABOLICFLUXBALANCE_H
#define METABOLICFLUXBALANCE_H

#include <string>

#include "kernel/simulator/ModelComponent.h"
#include "kernel/simulator/Plugin.h"
#include "plugins/data/BiochemicalSimulation/MetabolicNetwork.h"

/**
 * Component that evaluates a stub flux-balance objective over a MetabolicNetwork.
 */
class MetabolicFluxBalance : public ModelComponent {
public:
	MetabolicFluxBalance(Model* model, std::string name = "");
	virtual ~MetabolicFluxBalance() override = default;

public:
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord* fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

public:
	void setMetabolicNetwork(MetabolicNetwork* metabolicNetwork);
	MetabolicNetwork* getMetabolicNetwork() const;
	void setObjectiveReactionName(std::string objectiveReactionName);
	std::string getObjectiveReactionName() const;
	void setLastSucceeded(bool lastSucceeded);
	bool getLastSucceeded() const;
	void setLastObjectiveValue(double lastObjectiveValue);
	double getLastObjectiveValue() const;
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
		std::string metabolicNetworkName = "";
		std::string objectiveReactionName = "";
		bool lastSucceeded = false;
		double lastObjectiveValue = 0.0;
		std::string lastMessage = "";
	} DEFAULT;

	MetabolicNetwork* _metabolicNetwork = nullptr;
	std::string _objectiveReactionName = DEFAULT.objectiveReactionName;
	bool _lastSucceeded = DEFAULT.lastSucceeded;
	double _lastObjectiveValue = DEFAULT.lastObjectiveValue;
	std::string _lastMessage = DEFAULT.lastMessage;
};

#endif /* METABOLICFLUXBALANCE_H */
