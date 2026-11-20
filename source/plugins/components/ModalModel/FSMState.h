#pragma once

#include "DefaultNode.h"

class DefaultNetwork;

class FSMState : public DefaultNode {
public:
	FSMState(Model* model, std::string name = "");
	virtual ~FSMState() = default;

public:
	void setEntryActionExpression(std::string expression);
	std::string getEntryActionExpression() const;
	void setExitActionExpression(std::string expression);
	std::string getExitActionExpression() const;

    /** Optional non-owning refined Network; a null reference means no refinement. */
    void setRefinementNetwork(DefaultNetwork* network);
    DefaultNetwork* getRefinementNetwork() const;
    bool hasUnresolvedRefinementNetwork() const;
    std::string getRefinementNetworkIdentity() const;

public: // static
	static PluginInformation* GetPluginInformation();
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord *fields);
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
    mutable DefaultNetwork* _refinementNetwork = nullptr; // never owns target
    std::string _refinementNetworkClassname;
    std::string _refinementNetworkName;
};
