#include "plugins/components/ModalModel/ModalModelFSM.h"
#include "../../../kernel/simulator/model/Model.h"
#include "plugins/components/ModalModel/DefaultTransitionExtensions.h"
#include <vector>

namespace {
int NodeIndex(List<DefaultNode*>* nodes, DefaultNode* node) {
	if (nodes == nullptr || node == nullptr) {
		return -1;
	}
	for (unsigned int i = 0; i < nodes->size(); i++) {
		if (nodes->getAtRank(i) == node) {
			return static_cast<int>(i);
		}
	}
	return -1;
}

void ExecuteStateAction(Model* model, FSMState* state, bool entryAction) {
	if (model == nullptr || state == nullptr) {
		return;
	}
	const std::string expression = entryAction ? state->getEntryActionExpression() : state->getExitActionExpression();
	if (expression != "") {
		model->parseExpression(expression);
	}
}

} // namespace

#ifdef PLUGINCONNECT_DYNAMIC
extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &ModalModelFSM::GetPluginInformation;
}
#endif

ModalModelFSM::ModalModelFSM(Model* model, std::string name)
	: ModalModelDefault(model, Util::TypeOf<ModalModelFSM>(), name) {}

PluginInformation* ModalModelFSM::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<ModalModelFSM>(), &ModalModelFSM::LoadInstance, &ModalModelFSM::NewInstance);
	info->setCategory("ModalModel");
	info->setDescriptionHelp("Specialization of ModalModelDefault for finite-state machine style models.");
	return info;
}

ModelComponent* ModalModelFSM::LoadInstance(Model* model, PersistenceRecord *fields) {
	ModalModelFSM* component = new ModalModelFSM(model);
	component->_loadInstance(fields);
	return component;
}

ModelDataDefinition* ModalModelFSM::NewInstance(Model* model, std::string name) {
	return new ModalModelFSM(model, name);
}

DefaultNodeTransition* ModalModelFSM::_createTransitionForLoad(DefaultNode* source, DefaultNode* destination, const std::string& transitionTypename, const std::string& transitionName, PersistenceRecord* fields, const std::string& suffix) {
	(void) fields;
	(void) suffix;
	if (transitionTypename == Util::TypeOf<EFSMTransition>()) {
		return new EFSMTransition(source, destination, transitionName);
	}
	return ModalModelDefault::_createTransitionForLoad(source, destination, transitionTypename, transitionName, fields, suffix);
}

std::string ModalModelFSM::_getTransitionTypename(DefaultNodeTransition* transition) const {
	if (dynamic_cast<EFSMTransition*>(transition) != nullptr) {
		return Util::TypeOf<EFSMTransition>();
	}
	return ModalModelDefault::_getTransitionTypename(transition);
}

void ModalModelFSM::_loadTransitionSpecificFields(DefaultNodeTransition* transition, PersistenceRecord* fields, const std::string& suffix) {
	EFSMTransition* efsmTransition = dynamic_cast<EFSMTransition*>(transition);
	if (efsmTransition == nullptr) {
		return;
	}
	efsmTransition->setTriggerEvent(fields->loadField("transitionTriggerEvent" + suffix, transition->getInputEvent()));
}

void ModalModelFSM::_saveTransitionSpecificFields(DefaultNodeTransition* transition, PersistenceRecord* fields, const std::string& suffix, bool saveDefaultValues) {
	EFSMTransition* efsmTransition = dynamic_cast<EFSMTransition*>(transition);
	if (efsmTransition == nullptr) {
		return;
	}
	fields->saveField("transitionTriggerEvent" + suffix, efsmTransition->getTriggerEvent(), "", saveDefaultValues);
}

void ModalModelFSM::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	std::string currentNodeAttribute = "Entity.ModalModel." + getName() + ".CurrentNode";
	std::string lastNodeAttribute = "Entity.ModalModel." + getName() + ".LastNode";
	const std::string dispatchEvent = std::to_string(inputPortNumber);

	DefaultNode* localCurrentNode = nullptr;
	double index = entity->getAttributeValue(currentNodeAttribute);
	double lastNodeId = entity->getAttributeValue(lastNodeAttribute);
	if (index == 0.0 && lastNodeId == 0.0 && _entryNode != nullptr) {
		localCurrentNode = _entryNode;
	}
	else {
		unsigned int currentIdx = static_cast<unsigned int>(index);
		if (currentIdx < _nodes->size()) {
			localCurrentNode = _nodes->getAtRank(currentIdx);
		}
	}
	if (localCurrentNode == nullptr) {
		localCurrentNode = _entryNode;
		if (localCurrentNode == nullptr) {
			for (DefaultNode* node : *_nodes->list()) {
				if (node->isInitialNode()) {
					localCurrentNode = node;
					break;
				}
			}
		}
	}
	if (localCurrentNode == nullptr && _nodes->size() > 0) {
		localCurrentNode = _nodes->front();
	}
	if (localCurrentNode == nullptr) {
		traceError("ModalModelFSM \"" + getName() + "\" has no current state to dispatch");
		_parentModel->sendEntityToComponent(entity, this->getConnectionManager()->getFrontConnection());
		return;
	}

	int initialNodeIndex = NodeIndex(_nodes, localCurrentNode);
	if (initialNodeIndex >= 0) {
		entity->setAttributeValue(currentNodeAttribute, static_cast<double>(initialNodeIndex), "", true);
	}
	_currentNode = localCurrentNode;
	entity->setAttributeValue(lastNodeAttribute, static_cast<double>(localCurrentNode->getId()), "", true);

	unsigned int transitions = 0;
	while (transitions < _maxTransitionsPerDispatch) {
		traceSimulation(this, "Current FSM state is \"" + localCurrentNode->getName() + "\" and " + std::to_string(transitions) + " transitions fired so far", TraceManager::Level::L7_internal);

		if (localCurrentNode->isFinalNode()) {
			traceSimulation(this, "Current FSM state is final; no transition will fire", TraceManager::Level::L7_internal);
			break;
		}

		std::vector<DefaultNodeTransition*> enabled;
		List<DefaultNodeTransition*>* outgoing = localCurrentNode->getTransitions();
		for (DefaultNodeTransition* transition : *outgoing->list()) {
			if (transition->canFire(_parentModel, entity, dispatchEvent)) {
				enabled.push_back(transition);
			}
		}
		if (enabled.empty()) {
			traceSimulation(this, "No FSM transition is enabled to fire", TraceManager::Level::L7_internal);
			break;
		}

		DefaultNodeTransition* chosen = enabled.front();
		traceSimulation(this, "Transition \"" + chosen->getName() + "\" from \"" + chosen->getSource()->getName() + "\" to \"" + chosen->getDestination()->getName() + "\" fires.", TraceManager::Level::L7_internal);
		ExecuteStateAction(_parentModel, dynamic_cast<FSMState*>(localCurrentNode), false);
		chosen->execute(_parentModel, entity);
		transitions++;

		DefaultNode* nextNode = chosen->getDestination();
		ExecuteStateAction(_parentModel, dynamic_cast<FSMState*>(nextNode), true);
		localCurrentNode = nextNode;
		if (localCurrentNode == nullptr) {
			traceError("New FSM current state is unknown");
			break;
		}

		_currentNode = localCurrentNode;
		int nodeIndex = NodeIndex(_nodes, localCurrentNode);
		if (nodeIndex >= 0) {
			entity->setAttributeValue(currentNodeAttribute, static_cast<double>(nodeIndex), "", true);
		}
		entity->setAttributeValue(lastNodeAttribute, static_cast<double>(localCurrentNode->getId()), "", true);
		if (localCurrentNode->isFinalNode()) {
			traceSimulation(this, "Reached final FSM state \"" + localCurrentNode->getName() + "\"; no more transitions will fire", TraceManager::Level::L7_internal);
			break;
		}
	}

	traceSimulation(this, "Current FSM state is \"" + (localCurrentNode != nullptr ? localCurrentNode->getName() : "<none>") + "\" and " + std::to_string(transitions) + " transitions were fired, so moving on...", TraceManager::Level::L7_internal);
	double waitTime = _parentModel->parseExpression(_timeDelayExpressionPerDispatch);
	Util::TimeUnit stu = _parentModel->getSimulation()->getReplicationBaseTimeUnit();
	waitTime *= Util::TimeUnitConvert(_timeDelayPerDispatchTimeUnit, stu);

	const unsigned int outputPort = localCurrentNode != nullptr && localCurrentNode->isFinalNode() ? 1u : 0u;
	Connection* connection = this->getConnectionManager()->getConnectionAtPort(outputPort);
	if (connection == nullptr) {
		connection = this->getConnectionManager()->getFrontConnection();
	}
	_parentModel->sendEntityToComponent(entity, connection, waitTime);
}

bool ModalModelFSM::_check(std::string& errorMessage) {
	bool resultAll = true;
	resultAll &= ModalModelDefault::_check(errorMessage);
	if (getNodes()->size() == 0) {
		errorMessage += "ModalModelFSM requires at least one state node. ";
		resultAll = false;
	}

	bool hasInitialState = false;
	for (DefaultNode* node : *getNodes()->list()) {
		FSMState* state = dynamic_cast<FSMState*>(node);
		if (state == nullptr) {
			errorMessage += "ModalModelFSM node \"" + (node != nullptr ? node->getName() : "<null>") + "\" is not an FSMState. ";
			resultAll = false;
			continue;
		}
		if (state->isInitialNode()) {
			hasInitialState = true;
		}
		if (state->getEntryActionExpression() != "") {
			resultAll &= _parentModel->checkExpression(state->getEntryActionExpression(), "entry action[" + state->getName() + "]", errorMessage);
		}
		if (state->getExitActionExpression() != "") {
			resultAll &= _parentModel->checkExpression(state->getExitActionExpression(), "exit action[" + state->getName() + "]", errorMessage);
		}
	}
	if (!hasInitialState) {
		errorMessage += "ModalModelFSM requires at least one initial state. ";
		resultAll = false;
	}
	for (DefaultNodeTransition* transition : *getTransitions()->list()) {
		EFSMTransition* efsmTransition = dynamic_cast<EFSMTransition*>(transition);
		if (efsmTransition == nullptr) {
			errorMessage += "ModalModelFSM transition \"" + (transition != nullptr ? transition->getName() : "<null>") + "\" is not an EFSMTransition. ";
			resultAll = false;
			continue;
		}
		if (efsmTransition->getTriggerEvent() != "") {
			resultAll &= _parentModel->checkExpression(efsmTransition->getTriggerEvent(), "trigger event[" + transition->getName() + "]", errorMessage);
		}
	}
	return resultAll;
}

void ModalModelFSM::_initBetweenReplications() {

}
