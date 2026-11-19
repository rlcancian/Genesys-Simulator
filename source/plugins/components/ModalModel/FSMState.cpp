#include "plugins/components/ModalModel/FSMState.h"
#include "plugins/data/ModalModel/DefaultNetwork.h"
#include "kernel/simulator/model/ModelDataManager.h"
#include <stdexcept>
#include <utility>
#include "kernel/simulator/PluginInformation.h"
#include "kernel/simulator/SimulationControlAndResponse.h"
#include "kernel/simulator/model/Model.h"

namespace {
constexpr const char* kRefinementAssociation = "FSMState.RefinementNetwork";
using RefinementControlBase = SimulationControlGenericClass<DefaultNetwork*, Model*, DefaultNetwork>;

class RefinementNetworkControl final : public RefinementControlBase {
public:
    RefinementNetworkControl(Model* model, FSMState* state, NameResolver resolver, ChoicesProvider choices)
        : RefinementControlBase(model,
              [state]() { return state->getRefinementNetwork(); },
              [state](DefaultNetwork* network) { state->setRefinementNetwork(network); },
              Util::TypeOf<FSMState>(), state->getName(), "Refinement Network",
              "Optional existing DefaultNetwork executed when this EFSM state is active.",
              false, true, false, nullptr, true, false, std::move(resolver),
              std::move(choices), nullptr, true), _state(state) {}

    std::string getValue() const override { return _state->getRefinementNetworkIdentity(); }
    bool supportsObjectCreation() const override { return false; }
    bool createObjectInstance(const std::string& = "") override { return false; }
    bool createObjectInstanceOfType(const std::string&, const std::string& = "") override { return false; }
private:
    FSMState* _state;
};
} // namespace

FSMState::FSMState(Model* model, std::string name) : DefaultNode(model, Util::TypeOf<FSMState>(), name) {
	const std::string classname = Util::TypeOf<FSMState>();
	auto* entryAction = new SimulationControlGeneric<std::string>(
		[this]() { return getEntryActionExpression(); },
		[this](const std::string& value) { setEntryActionExpression(value); },
		classname, getName(), "Entry Action Expression", "Expression evaluated when entering this state.");
	auto* exitAction = new SimulationControlGeneric<std::string>(
		[this]() { return getExitActionExpression(); },
		[this](const std::string& value) { setExitActionExpression(value); },
		classname, getName(), "Exit Action Expression", "Expression evaluated when leaving this state.");
	auto* finalNode = new SimulationControlBool(
		[this]() { return isFinalNode(); },
		[this](bool value) { setFinalNode(value); },
		classname, getName(), "Final Node", "Whether this state is a final state.");
	auto* initialNode = new SimulationControlBool(
		[this]() { return isInitialNode(); },
		[this](bool value) { setInitialNode(value); },
		classname, getName(), "Initial Node", "Whether this state is the initial state of its Network.");
	_parentModel->getControls()->insert(entryAction);
	_addSimulationControl(entryAction);
	_parentModel->getControls()->insert(exitAction);
	_addSimulationControl(exitAction);
	_parentModel->getControls()->insert(finalNode);
	_addSimulationControl(finalNode);
	_parentModel->getControls()->insert(initialNode);
	_addSimulationControl(initialNode);

    auto resolver = [model](const std::string& choice) -> DefaultNetwork* {
        if (model == nullptr || model->getDataManager() == nullptr) return nullptr;
        DefaultNetwork* match = nullptr;
        for (DefaultNetwork* candidate : model->getDataManager()->getDataDefinitionsAssignableTo<DefaultNetwork>()) {
            if (candidate == nullptr) continue;
            const std::string typed = candidate->getClassname() + "::" + candidate->getName();
            if (choice == typed || choice == candidate->getName()) {
                if (match != nullptr) return nullptr; // never silently select an ambiguous name
                match = candidate;
            }
        }
        return match;
    };
    auto choices = [model]() {
        auto* names = new List<std::string>();
        if (model != nullptr && model->getDataManager() != nullptr)
            for (DefaultNetwork* candidate : model->getDataManager()->getDataDefinitionsAssignableTo<DefaultNetwork>())
                if (candidate != nullptr)
                    names->insert(candidate->getClassname() + "::" + candidate->getName());
        return names;
    };
    auto* refinementControl = new RefinementNetworkControl(_parentModel, this, resolver, choices);
    _parentModel->getControls()->insert(refinementControl);
    _addSimulationControl(refinementControl);
}

void FSMState::setEntryActionExpression(std::string expression) {
	_entryActionExpression = expression;
}

std::string FSMState::getEntryActionExpression() const {
	return _entryActionExpression;
}

void FSMState::setExitActionExpression(std::string expression) {
	_exitActionExpression = expression;
}

std::string FSMState::getExitActionExpression() const {
	return _exitActionExpression;
}

void FSMState::setRefinementNetwork(DefaultNetwork* network) {
    if (network != nullptr && network->getParentModel() != _parentModel)
        throw std::invalid_argument("FSMState refinement must belong to the same simulation model");
    _attachedDataRemove(kRefinementAssociation);
    _refinementNetwork = network;
    _refinementNetworkClassname = network != nullptr ? network->getClassname() : std::string{};
    _refinementNetworkName = network != nullptr ? network->getName() : std::string{};
    if (network != nullptr) _attachedDataInsert(kRefinementAssociation, network);
}

DefaultNetwork* FSMState::getRefinementNetwork() const {
    if (_parentModel == nullptr || _parentModel->getDataManager() == nullptr) return nullptr;
    for (DefaultNetwork* candidate : _parentModel->getDataManager()
             ->getDataDefinitionsAssignableTo<DefaultNetwork>()) {
        if (candidate == nullptr) continue;
        if (_refinementNetwork != nullptr && candidate == _refinementNetwork)
            return candidate;
        if (_refinementNetwork == nullptr && !_refinementNetworkClassname.empty()
            && candidate->getClassname() == _refinementNetworkClassname
            && candidate->getName() == _refinementNetworkName) {
            _refinementNetwork = candidate;
            return candidate;
        }
    }
    return nullptr;
}

bool FSMState::hasUnresolvedRefinementNetwork() const {
    return !_refinementNetworkName.empty() && getRefinementNetwork() == nullptr;
}

std::string FSMState::getRefinementNetworkIdentity() const {
    if (DefaultNetwork* network = getRefinementNetwork())
        return network->getClassname() + "::" + network->getName();
    if (_refinementNetworkName.empty()) return {};
    return _refinementNetworkClassname + "::" + _refinementNetworkName;
}

PluginInformation* FSMState::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<FSMState>(), &FSMState::LoadInstance, &FSMState::NewInstance);
	info->setCategory("ModalModel");
	info->setDescriptionHelp("FSM node/state with optional entry and exit action expressions.");
	info->setReceiveTransfer(true); // FSM nodes do not need to be connected from a source to a sink
	info->setSendTransfer(true); // FSM nodes do not need to be connected in a process flow
	return info;
}

ModelDataDefinition* FSMState::LoadInstance(Model* model, PersistenceRecord *fields) {
	// Prefer the canonical ModelDataManager instance when nested network fields and
	// top-level serialization both refer to the same named state.
	const std::string name = fields->loadField("name", std::string(""));
	if (model != nullptr && !name.empty()) {
		if (FSMState* existing = dynamic_cast<FSMState*>(
				model->getDataManager()->getDataDefinition(Util::TypeOf<FSMState>(), name))) {
			// Do not re-apply fields: a later top-level load must not wipe
			// associations already resolved by a nested network load.
			return existing;
		}
	}
	FSMState* component = new FSMState(model);
	component->_loadInstance(fields);
	return component;
}

ModelDataDefinition* FSMState::NewInstance(Model* model, std::string name) {
	return new FSMState(model, name);
}

bool FSMState::_loadInstance(PersistenceRecord *fields) {
	bool res = DefaultNode::_loadInstance(fields);
	if (res) {
		_entryActionExpression = fields->loadField("entryActionExpression", DEFAULT.entryActionExpression);
		_exitActionExpression = fields->loadField("exitActionExpression", DEFAULT.exitActionExpression);
        _refinementNetwork = nullptr;
        _refinementNetworkClassname = fields->loadField("refinementNetworkClassname", std::string{});
        _refinementNetworkName = fields->loadField("refinementNetworkName", std::string{});
	}
	return res;
}

void FSMState::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	DefaultNode::_saveInstance(fields, saveDefaultValues);
	fields->saveField("entryActionExpression", _entryActionExpression, DEFAULT.entryActionExpression, saveDefaultValues);
	fields->saveField("exitActionExpression", _exitActionExpression, DEFAULT.exitActionExpression, saveDefaultValues);
    DefaultNetwork* network = getRefinementNetwork();
    const std::string type = network != nullptr ? network->getClassname() : _refinementNetworkClassname;
    const std::string name = network != nullptr ? network->getName() : _refinementNetworkName;
    fields->saveField("refinementNetworkClassname", type, std::string{}, saveDefaultValues);
    fields->saveField("refinementNetworkName", name, std::string{}, saveDefaultValues);
}
