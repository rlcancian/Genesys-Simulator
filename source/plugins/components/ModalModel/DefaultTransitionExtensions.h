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

public:
	virtual bool canFire(Model* model, Entity* entity) const override;
	virtual void execute(Model* model, Entity* entity) const override;

private:
	std::string _triggerEvent = "";
	std::string _probabilityExpression = "";
};

