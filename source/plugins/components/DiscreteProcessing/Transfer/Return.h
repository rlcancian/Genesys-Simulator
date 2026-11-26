#pragma once
#include "../../../../kernel/simulator/model/ModelComponent.h"
class Return : public ModelComponent {
public:
    explicit Return(Model* model, std::string name = "");
    ~Return() override = default;
    static PluginInformation* GetPluginInformation();
    static ModelComponent* LoadInstance(Model* model, PersistenceRecord* fields);
    static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
protected:
    bool _check(std::string& errorMessage) override;
    void _createAttachedAttributes() override;
    void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override;
};
