#pragma once

#include "DefaultNode.h"
#include <string>

class EFSMTransition : public DefaultNodeTransition {
public:
	EFSMTransition(DefaultNode* source, DefaultNode* destination, std::string name = "");
	virtual ~EFSMTransition() = default;

public:
	void setTriggerEvent(std::string triggerEvent);
	std::string getTriggerEvent() const;
	void setProbabilityExpression(std::string probabilityExpression);
	std::string getProbabilityExpression() const;
	void setOutputActions(std::string actions);
	std::string getOutputActions() const;
	void setSetActions(std::string actions);
	std::string getSetActions() const;
	void setNondeterministic(bool enabled);
	bool isNondeterministic() const;
    void setImmediate(bool enabled);
    bool isImmediate() const;
    void setPreemptive(bool enabled);
    bool isPreemptive() const;
    void setHistory(bool enabled);
    bool isHistory() const;
    void setErrorTransition(bool enabled);
    bool isErrorTransition() const;
    void setTermination(bool enabled);
    bool isTermination() const;
	// Fallback relation: eligible only when no ordinary transition is enabled.
	void setDefaultTransition(bool enabled);
	bool isDefaultTransition() const;

public:
	virtual bool canFire(Model* model, Entity* entity) const override;
	virtual void execute(Model* model, Entity* entity) const override;

private:
	std::string _triggerEvent = "";
	std::string _probabilityExpression = "";
	bool _defaultTransition = false;
	bool _nondeterministic = false;
    bool _immediate = false;
    bool _preemptive = false;
    bool _history = false;
    bool _errorTransition = false;
    bool _termination = false;
	std::string _outputActions;
	std::string _setActions;
};

