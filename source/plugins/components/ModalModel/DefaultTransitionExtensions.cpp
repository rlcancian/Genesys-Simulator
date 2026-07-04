#include "plugins/components/ModalModel/DefaultTransitionExtensions.h"
#include "../../../kernel/simulator/model/Model.h"
#include <cerrno>
#include <cstdlib>

namespace {
bool IsNumericLiteral(const std::string& value) {
	if (value.empty()) {
		return false;
	}
	char* end = nullptr;
	errno = 0;
	std::strtod(value.c_str(), &end);
	return errno == 0 && end != value.c_str() && *end == '\0';
}

bool TriggerEventMatches(Model* model, const std::string& triggerEvent, const std::string& dispatchEvent) {
	if (triggerEvent == "") {
		return true;
	}
	bool eventMatched = triggerEvent == dispatchEvent;
	const bool numericTriggerEvent = IsNumericLiteral(triggerEvent);
	if (!eventMatched && numericTriggerEvent && IsNumericLiteral(dispatchEvent)) {
		eventMatched = std::strtod(triggerEvent.c_str(), nullptr) == std::strtod(dispatchEvent.c_str(), nullptr);
	}
	if (!eventMatched && !numericTriggerEvent && model != nullptr) {
		eventMatched = model->parseExpression(triggerEvent) != 0.0;
	}
	return eventMatched;
}
} // namespace

EFSMTransition::EFSMTransition(DefaultNode* source, DefaultNode* destination, std::string name)
	: DefaultNodeTransition(source, destination, name) {
}

void EFSMTransition::setTriggerEvent(std::string triggerEvent) {
	_triggerEvent = triggerEvent;
	setInputEvent(triggerEvent);
}

std::string EFSMTransition::getTriggerEvent() const {
	return _triggerEvent;
}

bool EFSMTransition::canFire(Model* model, Entity* entity) const {
	std::string dispatchEvent = "";
	if (model != nullptr && model->getSimulation() != nullptr && model->getSimulation()->getCurrentEvent() != nullptr) {
		dispatchEvent = std::to_string(model->getSimulation()->getCurrentEvent()->getComponentinputPortNumber());
	}
	return canFire(model, entity, dispatchEvent);
}

bool EFSMTransition::canFire(Model* model, Entity* entity, const std::string& dispatchEvent) const {
	const std::string triggerEvent = _triggerEvent != "" ? _triggerEvent : getInputEvent();
	if (!TriggerEventMatches(model, triggerEvent, dispatchEvent)) {
		return false;
	}
	bool parentCanFire = DefaultNodeTransition::canFire(model, entity, dispatchEvent);
	if (!parentCanFire) {
		return false;
	}
	return true;
}

void EFSMTransition::execute(Model* model, Entity* entity) const {
	DefaultNodeTransition::execute(model, entity);
}

PetriTransition::PetriTransition(DefaultNode* source, DefaultNode* destination, std::string name)
	: DefaultNodeTransition(source, destination, name) {
}

void PetriTransition::setInputArcWeight(std::string color, unsigned int weight) {
	_inputArcWeights[color] = weight;
}

void PetriTransition::setOutputArcWeight(std::string color, unsigned int weight) {
	_outputArcWeights[color] = weight;
}

bool PetriTransition::canFire(Model* model, Entity* entity) const {
	(void) model;
	(void) entity;
	PetriPlace* sourcePlace = dynamic_cast<PetriPlace*>(getSource());
	if (sourcePlace == nullptr) {
		return false;
	}
	for (const auto& pair : _inputArcWeights) {
		if (sourcePlace->getTokens(pair.first) < pair.second) {
			return false;
		}
	}
	return true;
}

void PetriTransition::execute(Model* model, Entity* entity) const {
	(void) model;
	(void) entity;
	PetriPlace* sourcePlace = dynamic_cast<PetriPlace*>(getSource());
	PetriPlace* destinationPlace = dynamic_cast<PetriPlace*>(getDestination());
	if (sourcePlace == nullptr || destinationPlace == nullptr) {
		return;
	}
	for (const auto& pair : _inputArcWeights) {
		sourcePlace->removeTokens(pair.second, pair.first);
	}
	for (const auto& pair : _outputArcWeights) {
		destinationPlace->addTokens(pair.second, pair.first);
	}
}
