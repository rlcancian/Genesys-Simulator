#include "plugins/components/Transfer/Label.h"
#include "plugins/components/Transfer/Route.h"
#include "plugins/components/Transfer/Goto.h"
#include "plugins/data/Transfer/Sequence.h"
#include "../../../kernel/simulator/model/Model.h"
#include <algorithm>

#ifdef PLUGINCONNECT_DYNAMIC
extern "C" StaticGetPluginInformation GetPluginInformation() {
    return &Label::GetPluginInformation;
}
#endif

namespace {
bool isRegistered(Model* model, LabelDefinition* candidate) {
    if (!model || !candidate) return false;
    const auto* data = model->getDataManager()->getDataDefinitionList(Util::TypeOf<LabelDefinition>())->list();
    return data && std::find(data->begin(), data->end(), candidate) != data->end();
}
}

Label::Label(Model* model, std::string name)
    : ModelComponent(model, Util::TypeOf<Label>(), name) {
    auto* prop = new SimulationControlGenericClass<LabelDefinition*, Model*, LabelDefinition>(
        _parentModel,
        std::bind(&Label::getLabelDefinition, this),
        std::bind(&Label::setLabelDefinition, this, std::placeholders::_1),
        Util::TypeOf<Label>(), getName(), "LabelDefinition", "");
    _parentModel->getControls()->insert(prop);
    _addSimulationControl(prop);

    // Creation is intentionally a one-time constructor action, not ModelCheck.
    setLabelDefinition(new LabelDefinition(model, getName() + ".Destination"));
}

Label::~Label() {
    // The definition is about to be destroyed with our internal-data map.
    // Detach all known non-owning consumers while the model registries still
    // contain them. Otherwise Route/SequenceStep can retain dangling pointers
    // after an interactive Label deletion.
    if (auto* definition = getLabelDefinition()) {
        for (auto* component : *_parentModel->getComponentManager()->getAllComponents()) {
            if (!component || component == this) continue;
            if (auto* sender = dynamic_cast<Goto*>(component)) {
                if (sender->getLabelDefinition() == definition)
                    sender->setLabelDefinition(nullptr);
            }
            if (auto* route = dynamic_cast<Route*>(component)) {
                if (route->getLabel() == definition)
                    route->setLabel(nullptr);
            }
        }
        for (const auto& type : _parentModel->getDataManager()->getDataDefinitionClassnames()) {
            if (type != Util::TypeOf<Sequence>()) continue;
            for (auto* item : *_parentModel->getDataManager()->getDataDefinitionList(type)->list()) {
                auto* sequence = dynamic_cast<Sequence*>(item);
                if (!sequence) continue;
                for (auto* step : *sequence->getSteps()->list()) {
                    if (step && step->getLabel() == definition)
                        step->setLabel(nullptr);
                }
            }
        }
        if (definition->getEnterIntoLabelComponent() == this)
            definition->setEnterIntoLabelComponent(nullptr);
    }
    // ModelDataDefinitionAssociations deletes our internal definition once.
}

PluginInformation* Label::GetPluginInformation() {
    auto* info = new PluginInformation(Util::TypeOf<Label>(), &Label::LoadInstance, &Label::NewInstance);
    info->setCategory("Transfer");
    info->setReceiveTransfer(true);
    info->setMinimumInputs(0);
    info->setMaximumInputs(0);
    info->setMinimumOutputs(1);
    info->setMaximumOutputs(1);
    info->insertDynamicLibFileDependence("labeldefinition.so");
    info->setDescriptionHelp("Receives transfer events at its internal LabelDefinition "
                             "and forwards the entity over one conventional connection.");
    return info;
}

ModelDataDefinition* Label::NewInstance(Model* model, std::string name) {
    return new Label(model, name);
}

ModelComponent* Label::LoadInstance(Model* model, PersistenceRecord* fields) {
    auto* component = new Label(model);
    try {
        component->_loadInstance(fields);
    } catch (const std::exception& e) {
        component->traceError("Failed to load Label component: " + std::string(e.what()));
    }
    return component;
}

std::string Label::show() {
    auto* definition = getLabelDefinition();
    return ModelComponent::show() +
           ",labelDefinition=" + (definition ? definition->getName() : _labelDefinitionName);
}

LabelDefinition* Label::getLabelDefinition() const {
    return isRegistered(_parentModel, _labelDefinition) ? _labelDefinition : nullptr;
}

void Label::setLabelDefinition(LabelDefinition* candidate) {
    if (!isRegistered(_parentModel, candidate)) {
        traceError("Label '" + getName() + "': LabelDefinition must exist in this model.");
        return;
    }
    // Internal ownership is exclusive. An existing receiver may not be stolen.
    auto* receiver = candidate->getEnterIntoLabelComponent();
    if (receiver && receiver != this) {
        traceError("Label '" + getName() + "': LabelDefinition '" + candidate->getName() +
                   "' is already assigned to another Label.");
        return;
    }
    for (auto* component : *_parentModel->getComponentManager()->getAllComponents()) {
        if (component != this && component && component->getInternalData("LabelDefinition") == candidate) {
            traceError("Label '" + getName() + "': LabelDefinition is internally owned by another component.");
            return;
        }
    }
    if (_labelDefinition == candidate) {
        _labelDefinitionName = candidate->getName();
        candidate->setEnterIntoLabelComponent(this);
        _mandatoryEditableDataDefinitionInsert("LabelDefinition", candidate);
        return;
    }
    // Do not destroy the former owned definition while some other model
    // element still holds an attached pointer to it. A rejected replacement
    // leaves the original association untouched.
    if (auto* old = getLabelDefinition()) {
        for (auto* component : *_parentModel->getComponentManager()->getAllComponents()) {
            if (component == this || !component) continue;
            // Route holds a direct destination pointer, not always attached data.
            if (auto* route = dynamic_cast<Route*>(component); route && route->getLabel() == old) {
                traceError("Label '" + getName() + "': cannot replace LabelDefinition referenced by Route '" +
                           route->getName() + "'.");
                return;
            }
            for (const auto& attached : *component->getAttachedData()) {
                if (attached.second == old) {
                    traceError("Label '" + getName() + "': cannot replace LabelDefinition while referenced by '" +
                               component->getName() + "'.");
                    return;
                }
            }
        }
        for (const auto& dataType : _parentModel->getDataManager()->getDataDefinitionClassnames()) {
            for (auto* owner : *_parentModel->getDataManager()->getDataDefinitionList(dataType)->list()) {
                if (!owner || owner == old) continue;
                // SequenceStep also stores a direct non-owning destination pointer.
                if (auto* sequence = dynamic_cast<Sequence*>(owner)) {
                    for (auto* step : *sequence->getSteps()->list()) {
                        if (step && step->getLabel() == old) {
                            traceError("Label '" + getName() +
                                       "': cannot replace LabelDefinition referenced by Sequence '" +
                                       sequence->getName() + "'.");
                            return;
                        }
                    }
                }
                for (const auto& attached : *owner->getAttachedData()) {
                    if (attached.second == old) {
                        traceError("Label '" + getName() + "': cannot replace referenced LabelDefinition '" +
                                   old->getName() + "'.");
                        return;
                    }
                }
            }
        }
    }
    // Internal association is the sole owner; it deletes the old target.
    _internalDataInsert("LabelDefinition", candidate);
    _labelDefinition = candidate;
    _labelDefinitionName = candidate->getName();
    candidate->setEnterIntoLabelComponent(this);
    _mandatoryEditableDataDefinitionInsert("LabelDefinition", candidate);
}

bool Label::_loadInstance(PersistenceRecord* fields) {
    if (!ModelComponent::_loadInstance(fields)) return false;
    _labelDefinitionName = fields->loadField("labelDefinition", "");
    if (_labelDefinitionName.empty()) return false;
    auto* candidate = dynamic_cast<LabelDefinition*>(
        _parentModel->getDataManager()->getDataDefinition(Util::TypeOf<LabelDefinition>(), _labelDefinitionName));
    if (candidate) setLabelDefinition(candidate);
    // During order-independent load, _createEditableDataDefinitions/_check
    // resolve this persisted name when the definition becomes available.
    return true;
}

void Label::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
    ModelComponent::_saveInstance(fields, saveDefaultValues);
    auto* definition = getLabelDefinition();
    fields->saveField("labelDefinition", definition ? definition->getName() : _labelDefinitionName);
}

void Label::_createEditableDataDefinitions() {
    if (!_labelDefinitionName.empty()) {
        auto* candidate = dynamic_cast<LabelDefinition*>(
            _parentModel->getDataManager()->getDataDefinition(Util::TypeOf<LabelDefinition>(), _labelDefinitionName));
        if (candidate && candidate != getLabelDefinition()) setLabelDefinition(candidate);
    }
    if (auto* definition = getLabelDefinition()) {
        _mandatoryEditableDataDefinitionInsert("LabelDefinition", definition);
        definition->setEnterIntoLabelComponent(this);
    } else {
        _mandatoryEditableDataDefinitionRemove("LabelDefinition");
    }
}

bool Label::_check(std::string& errorMessage) {
    _createEditableDataDefinitions();
    auto* definition = getLabelDefinition();
    if (!definition || getInternalData("LabelDefinition") != definition) {
        errorMessage += "Label '" + getName() + "': missing registered internal LabelDefinition. ";
        return false;
    }
    if (definition->getParentModel() != _parentModel ||
        definition->getEnterIntoLabelComponent() != this) {
        errorMessage += "Label '" + getName() + "': receiver association is invalid or conflicting. ";
        return false;
    }
    auto* connections = getConnectionManager();
    if (!connections || connections->size() != 1 || !connections->getFrontConnection() ||
        !connections->getFrontConnection()->component) {
        errorMessage += "Label '" + getName() + "': exactly one valid output connection is required. ";
        return false;
    }
    return true;
}

void Label::_onDispatchEvent(Entity* entity, unsigned int) {
    auto* connections = getConnectionManager();
    Connection* outgoing = connections ? connections->getFrontConnection() : nullptr;
    if (!entity || !outgoing || !outgoing->component) {
        traceError("Label '" + getName() + "': unable to forward entity without an output connection.");
        return;
    }
    _parentModel->sendEntityToComponent(entity, outgoing);
}
