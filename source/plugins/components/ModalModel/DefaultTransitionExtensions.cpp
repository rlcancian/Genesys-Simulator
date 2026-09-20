#include "plugins/components/ModalModel/DefaultTransitionExtensions.h"
#include "../../../kernel/simulator/model/Model.h"

EFSMTransition::EFSMTransition(DefaultNode* source, DefaultNode* destination, std::string name)
	: DefaultNodeTransition(source, destination, name) {
	setTransitionKind(TransitionKind::DETERMINISTIC);
}

void EFSMTransition::setTriggerEvent(std::string triggerEvent) {
	_triggerEvent = triggerEvent;
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

void EFSMTransition::execute(Model* model, Entity* entity) const {
	DefaultNodeTransition::execute(model, entity);
}
