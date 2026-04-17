#pragma once

#include "plugins/components/DiscreteProcessing/DefaultNode.h"

class FSMState : public DefaultNode {
public:
	FSMState(Model* model, std::string name = "");
	virtual ~FSMState() = default;

public:
	void setEntryActionExpression(std::string expression);
	std::string getEntryActionExpression() const;
	void setExitActionExpression(std::string expression);
	std::string getExitActionExpression() const;

public: // static
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

protected:
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;

private:
	const struct DEFAULT_VALUES {
		const std::string entryActionExpression = "";
		const std::string exitActionExpression = "";
	} DEFAULT;
	std::string _entryActionExpression = DEFAULT.entryActionExpression;
	std::string _exitActionExpression = DEFAULT.exitActionExpression;
};
