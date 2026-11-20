#pragma once

#include "../../../kernel/simulator/model/ModelDataDefinition.h"
#include "../../../kernel/simulator/essentialPlugins/Entity.h"

class ModelComponent;

/**
 * Logical transfer destination. The model owns this definition; its receiver
 * is a non-owning reference to one registered Label ModelComponent.
 */
class LabelDefinition : public ModelDataDefinition {
public:
    LabelDefinition(Model* model, std::string name = "");
    ~LabelDefinition() override = default;

    static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord* fields);
    static PluginInformation* GetPluginInformation();
    static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

    std::string show() override;

    /** Reject a second live receiver; nullptr detaches the current receiver. */
    void setEnterIntoLabelComponent(ModelComponent* enteringLabelComponent);
    /** Returns nullptr for a receiver no longer registered in this model. */
    ModelComponent* getEnterIntoLabelComponent() const;
    /** Schedules the original entity for a future Label dispatch event. */
    void sendEntityToLabelComponent(Entity* entity, double timeDelay);

protected:
    bool _loadInstance(PersistenceRecord* fields) override;
    void _saveInstance(PersistenceRecord* fields, bool saveDefaultValues) override;
    bool _check(std::string& errorMessage) override;

private:
    ModelComponent* _enteringLabelComponent = nullptr; // non-owning
};
