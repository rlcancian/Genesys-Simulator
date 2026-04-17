#pragma once

#include "plugins/components/DiscreteProcessing/DefaultNode.h"
#include "plugins/components/Network/PetriPlace.h"
#include <map>

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

class PetriTransition : public DefaultNodeTransition {
public:
	PetriTransition(DefaultNode* source, DefaultNode* destination, std::string name = "");
	virtual ~PetriTransition() = default;

public:
	void setInputArcWeight(std::string color, unsigned int weight);
	void setOutputArcWeight(std::string color, unsigned int weight);

public:
	virtual bool canFire(Model* model, Entity* entity) const override;
	virtual void execute(Model* model, Entity* entity) const override;

private:
	std::map<std::string, unsigned int> _inputArcWeights;
	std::map<std::string, unsigned int> _outputArcWeights;
};
