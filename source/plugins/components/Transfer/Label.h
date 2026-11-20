#pragma once

#include "../../../kernel/simulator/model/ModelComponent.h"
#include "../../data/Transfer/LabelDefinition.h"

/**
 * Receiving endpoint for transfer events. Owns one internal LabelDefinition
 * and forwards the same entity through its one conventional output.
 */
class Label : public ModelComponent {
public:
    explicit Label(Model* model, std::string name = "");
    ~Label() override;

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
    void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override;
    void _createEditableDataDefinitions() override;

private:
    LabelDefinition* _labelDefinition = nullptr;  // owned by internal-data association
    std::string _labelDefinitionName;           // persistent lookup, not an owning pointer
};
