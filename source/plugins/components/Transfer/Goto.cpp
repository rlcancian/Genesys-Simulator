#include "plugins/components/Transfer/Goto.h"
#include "../../../kernel/simulator/model/Model.h"
#include "kernel/simulator/Simulator.h"
#include <algorithm>
#include <cmath>

#ifdef PLUGINCONNECT_DYNAMIC
extern "C" StaticGetPluginInformation GetPluginInformation() {
    return &Goto::GetPluginInformation;
}
#endif

namespace {
bool registeredDestination(Model* model, const LabelDefinition* candidate) {
    if (!model || !candidate) return false;
    const auto* definitions = model->getDataManager()
        ->getDataDefinitionList(Util::TypeOf<LabelDefinition>())->list();
    return definitions &&
        std::find(definitions->begin(), definitions->end(), candidate) != definitions->end();
}
}

Goto::Goto(Model* model, std::string name)
    : ModelComponent(model, Util::TypeOf<Goto>(), name) {
    auto* propDestination = new SimulationControlGenericClass<LabelDefinition*, Model*, LabelDefinition>(
        _parentModel,
        std::bind(&Goto::getLabelDefinition, this),
        std::bind(&Goto::setLabelDefinition, this, std::placeholders::_1),
        Util::TypeOf<Goto>(), getName(), "LabelDefinition", "");
    auto* propExpression = new SimulationControlGeneric<std::string>(
        std::bind(&Goto::getTimeDelayExpression, this),
        std::bind(&Goto::setTimeDelayExpression, this, std::placeholders::_1),
        Util::TypeOf<Goto>(), getName(), "TimeDelayExpression", "");
    auto* propUnit = new SimulationControlGenericEnum<Util::TimeUnit, Util>(
        std::bind(&Goto::getTimeDelayTimeUnit, this),
        std::bind(&Goto::setTimeDelayTimeUnit, this, std::placeholders::_1),
        Util::TypeOf<Goto>(), getName(), "TimeDelayTimeUnit", "");

    for (auto* control : {static_cast<SimulationControl*>(propDestination),
                          static_cast<SimulationControl*>(propExpression),
                          static_cast<SimulationControl*>(propUnit)}) {
        _parentModel->getControls()->insert(control);
        _addSimulationControl(control);
    }
}

PluginInformation* Goto::GetPluginInformation() {
    auto* info = new PluginInformation(Util::TypeOf<Goto>(), &Goto::LoadInstance, &Goto::NewInstance);
    info->setCategory("Transfer");
    info->setSendTransfer(true);
    info->setMinimumInputs(1);
    info->setMaximumInputs(1);
    info->setMinimumOutputs(0);
    info->setMaximumOutputs(0);
    info->insertDynamicLibFileDependence("labeldefinition.so");
    info->setDescriptionHelp("Transfers the incoming entity to an existing LabelDefinition. "
                             "TimeDelayExpression is evaluated using the GenESyS parser.");
    return info;
}

ModelComponent* Goto::LoadInstance(Model* model, PersistenceRecord* fields) {
    auto* component = new Goto(model);
    try {
        component->_loadInstance(fields);
    } catch (const std::exception& e) {
        component->traceError("Failed to load Goto component: " + std::string(e.what()));
    }
    return component;
}

ModelDataDefinition* Goto::NewInstance(Model* model, std::string name) {
    return new Goto(model, name);
}

std::string Goto::show() {
    return ModelComponent::show() + ",labelDefinition=" + _destinationName
        + ",timeDelayExpression=" + _timeDelayExpression
        + ",timeDelayTimeUnit=" + Util::StrTimeUnitShort(_timeDelayTimeUnit);
}

void Goto::setLabelDefinition(LabelDefinition* candidate) {
    if (candidate && !registeredDestination(_parentModel, candidate)) {
        traceError("Goto '" + getName() + "': LabelDefinition does not belong to this model.");
        return;
    }
    _destination = candidate;
    _destinationName = candidate ? candidate->getName() : std::string{};
    if (candidate) _attachedDataInsert("LabelDefinition", candidate);
    else _attachedDataRemove("LabelDefinition");
}

LabelDefinition* Goto::getLabelDefinition() const {
    // Avoid dereferencing dangling raw pointers after target removal.
    return registeredDestination(_parentModel, _destination) ? _destination : nullptr;
}

void Goto::setTimeDelayExpression(const std::string& expression) {
    _timeDelayExpression = expression;
}
std::string Goto::getTimeDelayExpression() const { return _timeDelayExpression; }
void Goto::setTimeDelayTimeUnit(Util::TimeUnit unit) { _timeDelayTimeUnit = unit; }
Util::TimeUnit Goto::getTimeDelayTimeUnit() const { return _timeDelayTimeUnit; }

void Goto::_createEditableDataDefinitions() {
    if (getLabelDefinition() == nullptr && !_destinationName.empty()) {
        auto* recovered = dynamic_cast<LabelDefinition*>(
            _parentModel->getDataManager()->getDataDefinition(Util::TypeOf<LabelDefinition>(),
                                                             _destinationName));
        if (recovered) setLabelDefinition(recovered);
    }
    if (auto* destination = getLabelDefinition())
        _attachedDataInsert("LabelDefinition", destination);
    else
        _attachedDataRemove("LabelDefinition");
}

bool Goto::_loadInstance(PersistenceRecord* fields) {
    if (!ModelComponent::_loadInstance(fields)) return false;
    _destinationName = fields->loadField("labelDefinition", "");
    _destination = nullptr;
    _timeDelayExpression = fields->loadField("timeDelayExpression", DEFAULT.timeDelayExpression);
    _timeDelayTimeUnit = fields->loadField("timeDelayTimeUnit", DEFAULT.timeDelayTimeUnit);
    _createEditableDataDefinitions();
    return true;
}

void Goto::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
    ModelComponent::_saveInstance(fields, saveDefaultValues);
    const auto* destination = getLabelDefinition();
    fields->saveField("labelDefinition", destination ? destination->getName() : _destinationName);
    fields->saveField("timeDelayExpression", _timeDelayExpression,
                      DEFAULT.timeDelayExpression, saveDefaultValues);
    fields->saveField("timeDelayTimeUnit", _timeDelayTimeUnit,
                      DEFAULT.timeDelayTimeUnit, saveDefaultValues);
}

bool Goto::_check(std::string& errorMessage) {
    _createEditableDataDefinitions();
    auto* destination = getLabelDefinition();
    bool valid = destination && destination->getParentModel() == _parentModel;
    if (!valid) {
        errorMessage += "Goto '" + getName() + "': LabelDefinition is missing or unregistered. ";
    } else if (!destination->getEnterIntoLabelComponent()) {
        errorMessage += "Goto '" + getName() + "': LabelDefinition has no Label receiver. ";
        valid = false;
    }
    if (!_parentModel->checkExpression(_timeDelayExpression,
                                      getName() + ".TimeDelayExpression", errorMessage))
        valid = false;
    const auto baseUnit = _parentModel->getSimulation()->getReplicationBaseTimeUnit();
    const auto supported = [](Util::TimeUnit unit) {
        return static_cast<int>(unit) >= static_cast<int>(Util::TimeUnit::picosecond) &&
               static_cast<int>(unit) <= static_cast<int>(Util::TimeUnit::week);
    };
    if (!supported(_timeDelayTimeUnit) || !supported(baseUnit)) {
        errorMessage += "Goto '" + getName() + "': TimeDelayTimeUnit is invalid. ";
        valid = false;
    }
    if (getConnectionManager()->size() != 0) {
        errorMessage += "Goto '" + getName() + "': conventional outputs are forbidden. ";
        valid = false;
    }
    return valid;
}

void Goto::_onDispatchEvent(Entity* entity, unsigned int) {
    auto* destination = getLabelDefinition();
    if (!entity || !destination || !destination->getEnterIntoLabelComponent()) {
        traceError("Goto '" + getName() + "': no valid LabelDefinition/Label receiver.");
        return;
    }
    const auto baseUnit = _parentModel->getSimulation()->getReplicationBaseTimeUnit();
    const auto supported = [](Util::TimeUnit unit) {
        return static_cast<int>(unit) >= static_cast<int>(Util::TimeUnit::picosecond) &&
               static_cast<int>(unit) <= static_cast<int>(Util::TimeUnit::week);
    };
    if (!supported(_timeDelayTimeUnit) || !supported(baseUnit)) {
        traceError("Goto '" + getName() + "': invalid time unit.");
        return;
    }
    const double parsed = _parentModel->parseExpression(_timeDelayExpression);
    const double scale = Util::TimeUnitConvert(_timeDelayTimeUnit, baseUnit);
    const double delay = parsed * scale;
    if (!std::isfinite(parsed) || !std::isfinite(scale) || !std::isfinite(delay) || delay < 0.0) {
        traceError("Goto '" + getName() + "': TimeDelayExpression produced a negative or non-finite delay.");
        return;
    }
    destination->sendEntityToLabelComponent(entity, delay);
}
