#include "plugins/components/ModalModel/DefaultTransitionExtensions.h"
#include "../../../kernel/simulator/model/Model.h"

EFSMTransition::EFSMTransition(DefaultNode* source, DefaultNode* destination, std::string name)
	: DefaultNodeTransition(source, destination, name) {
	setTransitionKind(TransitionKind::DETERMINISTIC);
}

void EFSMTransition::setTriggerEvent(std::string triggerEvent) {
	_triggerEvent = triggerEvent;
	setInputEvent(triggerEvent);
}

std::string EFSMTransition::getTriggerEvent() const {
	return _triggerEvent;
}

void EFSMTransition::setProbabilityExpression(std::string probabilityExpression) {
	_probabilityExpression = probabilityExpression;
}

std::string EFSMTransition::getProbabilityExpression() const {
	return _probabilityExpression;
}

bool EFSMTransition::canFire(Model* model, Entity* entity) const {
	bool parentCanFire = DefaultNodeTransition::canFire(model, entity);
	if (!parentCanFire) {
		return false;
	}
	if (_probabilityExpression != "") {
		double p = model->parseExpression(_probabilityExpression);
		return p > 0.0;
	}
	return true;
}

bool EFSMTransition::canFire(Model* model, Entity* entity, const std::string& dispatchEvent) const {
	bool parentCanFire = DefaultNodeTransition::canFire(model, entity, dispatchEvent);
	if (!parentCanFire) {
		return false;
	}
	if (_probabilityExpression != "") {
		double p = model->parseExpression(_probabilityExpression);
		return p > 0.0;
	}
	return true;
}

void EFSMTransition::execute(Model* model, Entity* entity) const {
	DefaultNodeTransition::execute(model, entity);
}

double EFSMTransition::effectiveProbability(Model* model, Entity* entity) const {
	(void) entity;
	if (_probabilityExpression != "") {
		return model->parseExpression(_probabilityExpression);
	}
	return DefaultNodeTransition::effectiveProbability(model, entity);
}

PetriTransition::PetriTransition(DefaultNode* source, DefaultNode* destination, std::string name)
	: DefaultNodeTransition(source, destination, name) {
	//setTransitionKind(TransitionKind::PETRI);
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
