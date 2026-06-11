#pragma once

#include "plugins/components/ModalModel/ModalModelDefault.h"

class Entity;

class ModalModelPetriNet : public ModalModelDefault {
public:
    ModalModelPetriNet(Model* model, std::string name = "");
    virtual ~ModalModelPetriNet() = default;

public: // static
    static PluginInformation* GetPluginInformation();
    static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
    static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

protected:
    virtual bool _check(std::string& errorMessage) override;
    virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override;
    virtual void _initBetweenReplications() override;
};
