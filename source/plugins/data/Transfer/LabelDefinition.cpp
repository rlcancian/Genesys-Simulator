#include "plugins/data/Transfer/LabelDefinition.h"
#include "../../../kernel/simulator/model/Model.h"
#include <algorithm>

#ifdef PLUGINCONNECT_DYNAMIC
extern "C" StaticGetPluginInformation GetPluginInformation() {
    return &LabelDefinition::GetPluginInformation;
}
#endif

LabelDefinition::LabelDefinition(Model* model, std::string name)
    : ModelDataDefinition(model, Util::TypeOf<LabelDefinition>(), name) {}

ModelDataDefinition* LabelDefinition::NewInstance(Model* model, std::string name) {
    return new LabelDefinition(model, name);
}

ModelDataDefinition* LabelDefinition::LoadInstance(Model* model, PersistenceRecord* fields) {
    auto* result = new LabelDefinition(model);
    try {
        result->_loadInstance(fields);
    } catch (const std::exception& e) {
        result->traceError("Failed to load LabelDefinition instance: " + std::string(e.what()));
    }
    return result;
}

PluginInformation* LabelDefinition::GetPluginInformation() {
    auto* info = new PluginInformation(Util::TypeOf<LabelDefinition>(),
                                       &LabelDefinition::LoadInstance,
                                       &LabelDefinition::NewInstance);
    info->setCategory("Transfer");
    info->setDescriptionHelp("Logical transfer destination bound to exactly one Label component. "
                             "Multiple Goto and Route components can reference it.");
    return info;
}

std::string LabelDefinition::show() {
    std::string result = ModelDataDefinition::show();
    const auto* receiver = getEnterIntoLabelComponent();
    result += ",enteringComponent=" + (receiver ? receiver->getName() : std::string("NULL"));
    return result;
}

ModelComponent* LabelDefinition::getEnterIntoLabelComponent() const {
    if (!_enteringLabelComponent || !_parentModel || !_parentModel->getComponentManager()) {
        return nullptr;
    }
    const auto* registered = _parentModel->getComponentManager()->getAllComponents();
    if (!registered) return nullptr;
    return std::find(registered->begin(), registered->end(), _enteringLabelComponent)
           != registered->end() ? _enteringLabelComponent : nullptr;
}

void LabelDefinition::setEnterIntoLabelComponent(ModelComponent* candidate) {
    if (candidate == nullptr) {
        _enteringLabelComponent = nullptr;
        return;
    }
    const auto* registered = _parentModel->getComponentManager()->getAllComponents();
    if (!registered || std::find(registered->begin(), registered->end(), candidate) == registered->end()) {
        traceError("LabelDefinition '" + getName() + "': receiver is not registered in this model.");
        return;
    }
    const auto* previous = getEnterIntoLabelComponent();
    if (previous && previous != candidate) {
        traceError("LabelDefinition '" + getName() + "' already has receiver '" + previous->getName() + "'.");
        return;
    }
    _enteringLabelComponent = candidate;
}

void LabelDefinition::sendEntityToLabelComponent(Entity* entity, double timeDelay) {
    ModelComponent* receiver = getEnterIntoLabelComponent();
    if (!receiver || !entity) {
        traceError("LabelDefinition '" + getName() + "': entity cannot be transferred without a registered receiver.");
        return;
    }
    _parentModel->sendEntityToComponent(entity, receiver, timeDelay);
}

bool LabelDefinition::_loadInstance(PersistenceRecord* fields) {
    const bool loaded = ModelDataDefinition::_loadInstance(fields);
    // The Label component rebinds this reference after loading; do not resolve
    // components here because data definitions may be loaded before components.
    _enteringLabelComponent = nullptr;
    return loaded;
}

void LabelDefinition::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
    ModelDataDefinition::_saveInstance(fields, saveDefaultValues);
    // The receiver relationship is saved on Label, the owning component.
}

bool LabelDefinition::_check(std::string& errorMessage) {
    if (getEnterIntoLabelComponent()) return true;
    errorMessage += "LabelDefinition '" + getName() + "' has no registered Label receiver. ";
    return false;
}
