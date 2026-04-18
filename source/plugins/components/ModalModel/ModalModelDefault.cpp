/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   ModalModelDefault.cpp
 * Author: Prof. Rafael Luiz Cancian, Dr. Eng.
 * 
 * Created on 01 de Julho de 2025, 14:26
 */

#include "plugins/components/ModalModel/ModalModelDefault.h"
#include "kernel/simulator/Model.h"
#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/PluginManager.h"
#include <algorithm>
#include <cstdlib>
#include <memory>
#include <unordered_map>

#include "plugins/components/ModalModel/FSMState.h"
//#include "kernel/simulator/Simulator.h"
//#include "kernel/simulator/PluginManager.h"


/// Externalize function GetPluginInformation to be accessible through dynamic linked library
#ifdef PLUGINCONNECT_DYNAMIC
extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &ModalModelDefault::GetPluginInformation;
}
#endif


//
// public: /// constructors
//

ModalModelDefault::ModalModelDefault(Model* model, std::string name) : ModelComponent(
	model, Util::TypeOf<ModalModelDefault>(), name) {
}


//
// public: /// new public user methods for this component
//

void ModalModelDefault::addNode(DefaultNode* node) {
	if (node != nullptr && _nodes->find(node) == _nodes->list()->end()) {
		_nodes->insert(node);
	}
}

void ModalModelDefault::removeNode(DefaultNode* node) {
	_nodes->remove(node);
}

void ModalModelDefault::addTransition(DefaultNodeTransition* transition) {
	if (transition != nullptr && _transitions->find(transition) == _transitions->list()->end()) {
		_transitions->insert(transition);
		if (transition->getSource() != nullptr) {
			transition->getSource()->addTransition(transition);
		}
	}
}

void ModalModelDefault::removeTransition(DefaultNodeTransition* transition) {
	_transitions->remove(transition);
	if (transition != nullptr && transition->getSource() != nullptr) {
		transition->getSource()->removeTransition(transition);
	}
}

List<DefaultNode*>* ModalModelDefault::getNodes() const {
	return _nodes;
}

List<DefaultNodeTransition*>* ModalModelDefault::getTransitions() const {
	return _transitions;
}


void ModalModelDefault::setMaxTransitionsPerDispatch(unsigned int maxTransitionsPerDispatch) {
	_maxTransitionsPerDispatch = maxTransitionsPerDispatch;
}

unsigned int ModalModelDefault::getMaxTransitionsPerDispatch() const {
	return _maxTransitionsPerDispatch;
}


//
// public: /// virtual methods
//

std::string ModalModelDefault::show() {
	return ModelComponent::show() + "";
}


//
// public: /// static methods that must have implementations (Load and New just the same. GetInformation must provide specific infos for the new component
//

PluginInformation* ModalModelDefault::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<ModalModelDefault>(), &ModalModelDefault::LoadInstance,
	                                                &ModalModelDefault::NewInstance);
	info->setCategory("Network");
	//info->setMinimumInputs(1);
	//info->setMinimumOutputs(1);
	//info->setMaximumInputs(1);
	info->setMaximumOutputs(2); //[0]: normal output after every reaction; [1]: final output after finishing condition
	//info->setSource(false);
	//info->setSink(false);
	//info->setSendTransfer(false);
	//info->setReceiveTransfer(false);
	//info->insertDynamicLibFileDependence("...");
	info->setDescriptionHelp(
		"Represents an aggregate modal model made of nodes and transitions. It can be specialized for FSM and Petri-net style execution.");
	//info->setAuthor("...");
	//info->setDate("...");
	//info->setObservation("...");
	return info;
}

ModelComponent* ModalModelDefault::LoadInstance(Model* model, PersistenceRecord* fields) {
	ModalModelDefault* newComponent = new ModalModelDefault(model);
	try {
		newComponent->_loadInstance(fields);
	}
	catch (const std::exception& e) {
	}
	return newComponent;
}

ModelDataDefinition* ModalModelDefault::NewInstance(Model* model, std::string name) {
	return new ModalModelDefault(model, name);
}

void ModalModelDefault::addOutputExpressionReference(ModelDataDefinition* expressionReference) {
	_attachedDataInsert(expressionReference->getName(), expressionReference);
}

void ModalModelDefault::removeOutputExpressionReference(DefaultNodeTransition* expressionReference) {
	_attachedDataRemove(expressionReference->getName());
}


//
// protected: /// virtual method that must be overridden
//

bool ModalModelDefault::_loadInstance(PersistenceRecord* fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		_maxTransitionsPerDispatch = fields->loadField("maxTransitionsPerDispatch", DEFAULT.maxTransitionsPerDispatch);
		_timeDelayExpressionPerDispatch = fields->loadField("timeDelayExpressionPerDispatch", DEFAULT.timeDelayExpressionPerDispatch);
		_timeDelayPerDispatchTimeUnit = fields->loadField("timeDelayPerDispatchTimeUnit", DEFAULT.timeDelayPerDispatchTimeUnit);

		_nodes->clear();
		_transitions->clear();
		_entryNode = nullptr;
		_currentNode = nullptr;

		PluginManager* plugins = _parentModel->getParentSimulator()->getPluginManager();
		std::unordered_map<std::string, DefaultNode*> nodesByName;
		unsigned int nodesSize = fields->loadField("nodesSize", 0u);
		for (unsigned int i = 0; i < nodesSize; i++) {
			const std::string prefix = "node" + Util::StrIndex(i) + ".";
			auto nodeFields = std::unique_ptr<PersistenceRecord>(fields->newInstance());
			for (auto it = fields->begin(); it != fields->end(); ++it) {
				if (it->first.rfind(prefix, 0) == 0) {
					PersistenceRecord::Entry entry = it->second;
					entry.first = it->first.substr(prefix.size());
					nodeFields->insert(entry);
				}
			}
			if (nodeFields->size() == 0) {
				continue;
			}

			std::string nodeType = nodeFields->loadField("typename", Util::TypeOf<DefaultNode>());
			Plugin* nodePlugin = plugins->find(nodeType);
			if (nodePlugin == nullptr) {
				traceError("Could not load node plugin \"" + nodeType + "\" while loading ModalModelDefault \"" + getName() + "\"");
				continue;
			}

			ModelDataDefinition* loaded = nodePlugin->loadNew(_parentModel, nodeFields.get());
			DefaultNode* node = dynamic_cast<DefaultNode*>(loaded);
			if (node == nullptr) {
				traceError("Loaded modal node is not a DefaultNode for typename \"" + nodeType + "\"");
				continue;
			}
			node->setModelLevel(_id);
			addNode(node);
			nodesByName[node->getName()] = node;
		}

		unsigned int transitionsSize = fields->loadField("transitionsSize", 0u);
		for (unsigned int i = 0; i < transitionsSize; i++) {
			const std::string suffix = Util::StrIndex(i);
			std::string sourceName = fields->loadField("transitionSource" + suffix, "");
			std::string destinationName = fields->loadField("transitionDestination" + suffix, "");
			auto sourceIt = nodesByName.find(sourceName);
			auto destinationIt = nodesByName.find(destinationName);
			if (sourceIt == nodesByName.end() || destinationIt == nodesByName.end()) {
				traceError("Skipping modal transition with unknown source/destination while loading \"" + getName() + "\"");
				continue;
			}
			DefaultNodeTransition* transition = new DefaultNodeTransition(sourceIt->second, destinationIt->second,
				fields->loadField("transitionName" + suffix, "T" + suffix));
			transition->setGuardExpression(fields->loadField("transitionGuard" + suffix, ""));
			transition->setOutputExpression(fields->loadField("transitionOutput" + suffix, ""));
			transition->setInputEvent(fields->loadField("transitionInputEvent" + suffix, ""));
			transition->setPriority(fields->loadField("transitionPriority" + suffix, 0u));
			transition->setProbability(fields->loadField("transitionProbability" + suffix, 1.0));
			transition->setTransitionKind(static_cast<DefaultNodeTransition::TransitionKind>(
				fields->loadField("transitionKind" + suffix, static_cast<int>(DefaultNodeTransition::TransitionKind::DETERMINISTIC))));
			addTransition(transition);
		}

		std::string entryNodeName = fields->loadField("entryNode", "");
		if (entryNodeName != "") {
			auto entryIt = nodesByName.find(entryNodeName);
			if (entryIt != nodesByName.end()) {
				_entryNode = entryIt->second;
			}
		}
		if (_entryNode == nullptr && _nodes->size() > 0) {
			_entryNode = _nodes->front();
		}
		_currentNode = _entryNode;
	}
	return res;
}

void ModalModelDefault::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("maxTransitionsPerDispatch", _maxTransitionsPerDispatch, DEFAULT.maxTransitionsPerDispatch,
	                  saveDefaultValues);
	fields->saveField("timeDelayExpressionPerDispatch", _timeDelayExpressionPerDispatch,
	                  DEFAULT.timeDelayExpressionPerDispatch, saveDefaultValues);
	fields->saveField("timeDelayPerDispatchTimeUnit", _timeDelayPerDispatchTimeUnit,
	                  DEFAULT.timeDelayPerDispatchTimeUnit, saveDefaultValues);
	if (_entryNode != nullptr) {
		fields->saveField("entryNode", _entryNode->getName(), "", saveDefaultValues);
	}

	fields->saveField("nodesSize", _nodes->size(), 0u, saveDefaultValues);
	unsigned int i = 0;
	for (DefaultNode* node : *_nodes->list()) {
		const std::string prefix = "node" + Util::StrIndex(i) + ".";
		auto nodeFields = std::unique_ptr<PersistenceRecord>(fields->newInstance());
		ModelComponent::SaveInstance(nodeFields.get(), node);
		for (auto it = nodeFields->begin(); it != nodeFields->end(); ++it) {
			PersistenceRecord::Entry entry = it->second;
			entry.first = prefix + it->first;
			fields->insert(entry);
		}
		i++;
	}

	fields->saveField("transitionsSize", _transitions->size(), 0u, saveDefaultValues);
	i = 0;
	for (DefaultNodeTransition* transition : *_transitions->list()) {
		const std::string suffix = Util::StrIndex(i);
		if (transition->getSource() != nullptr) {
			fields->saveField("transitionSource" + suffix, transition->getSource()->getName(), "", saveDefaultValues);
		}
		if (transition->getDestination() != nullptr) {
			fields->saveField("transitionDestination" + suffix, transition->getDestination()->getName(), "", saveDefaultValues);
		}
		fields->saveField("transitionName" + suffix, transition->getName(), "", saveDefaultValues);
		fields->saveField("transitionGuard" + suffix, transition->getGuardExpression(), "", saveDefaultValues);
		fields->saveField("transitionOutput" + suffix, transition->getOutputExpression(), "", saveDefaultValues);
		fields->saveField("transitionInputEvent" + suffix, transition->getInputEvent(), "", saveDefaultValues);
		fields->saveField("transitionPriority" + suffix, transition->getPriority(), 0u, saveDefaultValues);
		fields->saveField("transitionProbability" + suffix, transition->getProbability(), 1.0, saveDefaultValues);
		fields->saveField("transitionKind" + suffix, static_cast<int>(transition->getTransitionKind()),
		                  static_cast<int>(DefaultNodeTransition::TransitionKind::DETERMINISTIC), saveDefaultValues);
		i++;
	}
}

bool ModalModelDefault::_check(std::string& errorMessage) {
	bool resultAll = true;
	for (auto transition : *_transitions->list()) {
		std::string guard = transition->getGuardExpression();
		if (guard != "") {
			resultAll &= _parentModel->checkExpression(guard, "guard expression[" + transition->getName() + "]",
			                                           errorMessage);
		}
		std::string output = transition->getOutputExpression();
		if (output != "") {
			resultAll &= _parentModel->checkExpression(output, "output expression[" + transition->getName() + "]",
			                                           errorMessage);
		}
	}
	return resultAll;
}

void ModalModelDefault::_initBetweenReplications() {
}

void ModalModelDefault::_createInternalAndAttachedData() {
	std::string currentNodeAttribute = "Entity.ModalModel." + getName() + ".CurrentNode";
	std::string lastNodeAttribute = "Entity.ModalModel." + getName() + ".LastNode";
	_attachedAttributesInsert({currentNodeAttribute, lastNodeAttribute});
}

std::string ModalModelDefault::getTimeDelayExpressionPerDispatch() {
	return _timeDelayExpressionPerDispatch;
}

void ModalModelDefault::setTimeDelayExpressionPerDispatch(const std::string time_delay_expression_per_dispatch) {
	_timeDelayExpressionPerDispatch = time_delay_expression_per_dispatch;
}

Util::TimeUnit ModalModelDefault::getTimeDelayPerDispatchTimeUnit() {
	return _timeDelayPerDispatchTimeUnit;
}

void ModalModelDefault::setTimeDelayPerDispatchTimeUnit(const Util::TimeUnit time_delay_per_dispatch_time_unit) {
	_timeDelayPerDispatchTimeUnit = time_delay_per_dispatch_time_unit;
}

void ModalModelDefault::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	(void)inputPortNumber;

	std::string currentNodeAttribute = "Entity.ModalModel." + getName() + ".CurrentNode";
	std::string lastNodeAttribute = "Entity.ModalModel." + getName() + ".LastNode";

	DefaultNode* localCurrentNode = nullptr;
	double index = entity->getAttributeValue(currentNodeAttribute);
	unsigned int currentIdx = static_cast<unsigned int>(index);
	if (currentIdx < _nodes->size()) {
		localCurrentNode = _nodes->getAtRank(currentIdx);
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
	unsigned int transitions= 0;
	while (transitions < _maxTransitionsPerDispatch) {
		traceSimulation(this, "Current node is \"" + localCurrentNode->getName()+"\" and "+std::to_string(transitions)+" transitions fired sor far", TraceManager::Level::L7_internal);

		List<DefaultNodeTransition*>* outgoing = localCurrentNode->getTransitions();
		std::vector<DefaultNodeTransition*> enabled;
		for (DefaultNodeTransition* transition : *outgoing->list()) {
			if (transition->canFire(_parentModel, entity)) {
				enabled.push_back(transition);
			}
		}
		if (enabled.size() == 0) {
			traceSimulation(this, "No transition is enabled to fire", TraceManager::Level::L7_internal);
			break;
		}
		else {
			std::sort(enabled.begin(), enabled.end(), [](DefaultNodeTransition* a, DefaultNodeTransition* b) {
				return a->getPriority() < b->getPriority();
			});
			DefaultNodeTransition* chosen = enabled.front();
			if (chosen->getTransitionKind() == DefaultNodeTransition::TransitionKind::PROBABILISTIC) {
				double probabilitySum = 0.0;
				for (DefaultNodeTransition* option : enabled) {
					probabilitySum += option->getProbability();
				}
				if (probabilitySum > 0.0) {
					double sample = (static_cast<double>(std::rand()) / static_cast<double>(RAND_MAX)) * probabilitySum;
					double accum = 0.0;
					for (DefaultNodeTransition* option : enabled) {
						accum += option->getProbability();
						if (sample <= accum) {
							chosen = option;
							break;
						}
					}
				}
			}
			traceSimulation(this, "Transition \"" + chosen->getName() + "\" from \"" + chosen->getSource()->getName() + "\" to \"" +chosen->getDestination()->getName() + "\" fires.", TraceManager::Level::L7_internal);
			chosen->execute(_parentModel, entity);
			transitions++;
			DefaultNode* nextNode = chosen->getDestination();
			localCurrentNode = nextNode;
			if (localCurrentNode != nullptr) {
				for (unsigned int i = 0; i < _nodes->size(); i++) {
					if (_nodes->getAtRank(i) == localCurrentNode) {
						entity->setAttributeValue(currentNodeAttribute, static_cast<double>(i));
						break;
					}
				}
				entity->setAttributeValue(lastNodeAttribute, static_cast<double>(localCurrentNode->getId()));
			}
			else {
				traceError("New current node is unknown");
			}
		}
	}
	traceSimulation(this, "Current node is \"" + localCurrentNode->getName()+"\" and "+std::to_string(transitions)+" transitions were fired, so moving on...", TraceManager::Level::L7_internal);
	double waitTime = _parentModel->parseExpression(_timeDelayExpressionPerDispatch);
	Util::TimeUnit stu = _parentModel->getSimulation()->getReplicationBaseTimeUnit(); //getReplicationLengthTimeUnit();
	waitTime *= Util::TimeUnitConvert(_timeDelayPerDispatchTimeUnit, stu);

	_parentModel->sendEntityToComponent(entity, this->getConnectionManager()->getFrontConnection(), waitTime);
}

DefaultNode* ModalModelDefault::getCurrentNode() {
	return _currentNode;
}

DefaultNode* ModalModelDefault::getEntryNode() {
	return _entryNode;
}

void ModalModelDefault::setEntryNode(DefaultNode* const entry_node) {
	_entryNode = entry_node;
}

//
// protected: /// virtual methods that could be overriden by derived classes, if needed
//

/*
bool ModalModelDefault::_check(std::string& errorMessage) {
	bool resultAll = true;
	resultAll &= _someString != "";
	resultAll &= _someUint > 0;
	return resultAll;
}
*/

/*
ParserChangesInformation* ModalModelDefault::_getParserChangesInformation() {
	ParserChangesInformation* changes = new ParserChangesInformation();
	//@TODO not implemented yet
	changes->getassignments().append("");
	changes->getexpressionProductions().append("");
	changes->getexpressions().append("");
	changes->getfunctionProductions().append("");
	changes->getassignments().append("");
	changes->getincludes().append("");
	changes->gettokens().append("");
	changes->gettypeObjs().append("");
	return changes;
}
*/

/*
void ModalModelDefault::_initBetweenReplications() {
	_someString = "Test";
	_someUint = 1;
}
*/

/*
void ModalModelDefault::_createInternalAndAttachedData() {
	if (_internalDataDefinition == nullptr) {
		PluginManager* pm = _parentModel->getParentSimulator()->getPlugins();
		_internalDataDefinition = pm->newInstance<DummyElement>(_parentModel, getName() + "." + "JustaDummy");
		_internalDataInsert("JustaDummy", _internalDataDefinition);
	}
	if (_attachedDataDefinition == nullptr) {
		PluginManager* pm = _parentModel->getParentSimulator()->getPlugins();
		_attachedDataDefinition = pm->newInstance<DummyElement>(_parentModel);
		_attachedDataInsert("JustaDummy", _attachedDataDefinition);
	}
}
*/

/*
void ModalModelDefault::_addProperty(SimulationControl* property) {
}
*/
