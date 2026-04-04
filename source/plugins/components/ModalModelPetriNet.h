#pragma once

#include "DefaultModalModel.h"

class ModalModelPetriNet : public DefaultModalModel {
public:
	ModalModelPetriNet(Model* model, std::string name = "");
	virtual ~ModalModelPetriNet() = default;

public: // static
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

protected:
	virtual bool _check(std::string& errorMessage) override;
};
