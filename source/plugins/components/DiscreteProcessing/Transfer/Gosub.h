#pragma once

#include "../../../../kernel/simulator/model/ModelComponent.h"
#include "../../../data/DiscreteProcessing/Transfer/LabelDefinition.h"

class Gosub : public ModelComponent {
public:
    explicit Gosub(Model* model, std::string name = "");
    ~Gosub() override = default;
    static PluginInformation* GetPluginInformation();
    static ModelComponent* LoadInstance(Model* model, PersistenceRecord* fields);
    static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
    std::string show() override;
    LabelDefinition* getLabelDefinition() const;
    void setLabelDefinition(LabelDefinition* definition);
protected:
    bool _loadInstance(PersistenceRecord* fields) override;
    void _saveInstance(PersistenceRecord* fields, bool saveDefaultValues) override;
    bool _check(std::string& errorMessage) override;
    void _createAttachedAttributes() override;
    void _createEditableDataDefinitions() override;
    void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override;
private:
    LabelDefinition* _destination = nullptr;
    std::string _destinationName;
};
