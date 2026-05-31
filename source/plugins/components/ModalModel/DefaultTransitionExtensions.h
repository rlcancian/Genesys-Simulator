#pragma once

#include "DefaultNode.h"
#include "plugins/components/ModalModel/PetriPlace.h"
#include <map>
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

using ColorWeightMap = std::map<std::string, unsigned int>;

class PetriTransition : public DefaultNodeTransition {
public:
	PetriTransition(DefaultNode* source, DefaultNode* destination, std::string name = "");
	virtual ~PetriTransition() = default;

public:
    void setInputArcWeight(PetriPlace* place, std::string color, unsigned int weight);
    unsigned int getInputArcWeight(PetriPlace* place, std::string color);
    void setOutputArcWeight(PetriPlace* place, std::string color, unsigned int weight);
    unsigned int getOutputArcWeight(PetriPlace* place, std::string color);

    std::map<PetriPlace*, ColorWeightMap> getInputPlaces() const;
    std::map<PetriPlace*, ColorWeightMap> getOutputPlaces() const;

    virtual bool canFire(Model* model, Entity* entity) const override;
	virtual void execute(Model* model, Entity* entity) const override;

    virtual bool _loadInstance(PersistenceRecord *fields, Model* model);
    virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues);

// private:
// 	std::map<std::string, unsigned int> _inputArcWeights;
// 	std::map<std::string, unsigned int> _outputArcWeights;

protected:
    std::map<PetriPlace*, ColorWeightMap> _inputPlaces;
    std::map<PetriPlace*, ColorWeightMap> _outputPlaces;
};
