#pragma once

#include "../../../kernel/simulator/model/ModelComponent.h"
#include "../../data/Transfer/LabelDefinition.h"

/** Transfers its incoming entity to a LabelDefinition without a conventional output. */
class Goto : public ModelComponent {
public:
    explicit Goto(Model* model, std::string name = "");
    ~Goto() override = default;

    static PluginInformation* GetPluginInformation();
    static ModelComponent* LoadInstance(Model* model, PersistenceRecord* fields);
    static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

    std::string show() override;
    void setLabelDefinition(LabelDefinition* definition);
    LabelDefinition* getLabelDefinition() const;
    void setTimeDelayExpression(const std::string& expression);
    std::string getTimeDelayExpression() const;
    void setTimeDelayTimeUnit(Util::TimeUnit unit);
    Util::TimeUnit getTimeDelayTimeUnit() const;

protected:
    bool _loadInstance(PersistenceRecord* fields) override;
    void _saveInstance(PersistenceRecord* fields, bool saveDefaultValues) override;
    bool _check(std::string& errorMessage) override;
    void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override;
    void _createEditableDataDefinitions() override;

private:
    const struct {
        const std::string timeDelayExpression = "0.0";
        const Util::TimeUnit timeDelayTimeUnit = Util::TimeUnit::second;
    } DEFAULT;

    LabelDefinition* _destination = nullptr; // attached, never owned
    std::string _destinationName;
    std::string _timeDelayExpression = DEFAULT.timeDelayExpression;
    Util::TimeUnit _timeDelayTimeUnit = DEFAULT.timeDelayTimeUnit;
};
