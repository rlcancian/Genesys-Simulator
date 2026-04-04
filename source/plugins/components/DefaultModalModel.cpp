/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   DefaultModalModel.cpp
 * Author: Prof. Rafael Luiz Cancian, Dr. Eng.
 * 
 * Created on 01 de Julho de 2025, 14:26
 */

#include "DefaultModalModel.h"
#include "../../kernel/simulator/Model.h"
#include <algorithm>
#include <cstdlib>
//#include "../../kernel/simulator/Simulator.h"
//#include "../../kernel/simulator/PluginManager.h"



/// Externalize function GetPluginInformation to be accessible throught dynamic linked library
#ifdef PLUGINCONNECT_DYNAMIC
extern "C" StaticGetPluginInformation GetPluginInformation() {
    return &DefaultModalModel::GetPluginInformation;
}
#endif


//
// public: /// constructors
//

DefaultModalModel::DefaultModalModel(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<DefaultModalModel>(), name) {
}


//
// public: /// new public user methods for this component
//

void DefaultModalModel::addNode(DefaultNode* node){
	if (node != nullptr && _nodes->find(node) == _nodes->list()->end()) {
		_nodes->insert(node);
	}
}
void DefaultModalModel::removeNode(DefaultNode* node){
	_nodes->remove(node);
}
void DefaultModalModel::addTransition(DefaultNodeTransition* transition){
	if (transition != nullptr && _transitions->find(transition) == _transitions->list()->end()) {
		_transitions->insert(transition);
		if (transition->getSource() != nullptr) {
			transition->getSource()->addTransition(transition);
		}
	}
}
void DefaultModalModel::removeTransition(DefaultNodeTransition* transition){
	_transitions->remove(transition);
	if (transition != nullptr && transition->getSource() != nullptr) {
		transition->getSource()->removeTransition(transition);
	}
}

List<DefaultNode*>* DefaultModalModel::getNodes() const {
	return _nodes;
}

List<DefaultNodeTransition*>* DefaultModalModel::getTransitions() const {
	return _transitions;
}

void DefaultModalModel::setEntryNode(DefaultNode* entryNode) {
	_entryNodeName = (entryNode != nullptr ? entryNode->getName() : "");
}

DefaultNode* DefaultModalModel::getEntryNode() const {
	if (_entryNodeName == "") {
		return nullptr;
	}
	for (DefaultNode* node : *_nodes->list()) {
		if (node->getName() == _entryNodeName) {
			return node;
		}
	}
	return nullptr;
}

void DefaultModalModel::setMaxTransitionsPerDispatch(unsigned int maxTransitionsPerDispatch) {
	_maxTransitionsPerDispatch = maxTransitionsPerDispatch;
}

unsigned int DefaultModalModel::getMaxTransitionsPerDispatch() const {
	return _maxTransitionsPerDispatch;
}


//
// public: /// virtual methods
//

std::string DefaultModalModel::show() {
	return ModelComponent::show() + "";
}


//
// public: /// static methods that must have implementations (Load and New just the same. GetInformation must provide specific infos for the new component
//

PluginInformation* DefaultModalModel::GetPluginInformation() {
    PluginInformation* info = new PluginInformation(Util::TypeOf<DefaultModalModel>(), &DefaultModalModel::LoadInstance, &DefaultModalModel::NewInstance);
	info->setCategory("Network");
	//info->setMinimumInputs(1);
	//info->setMinimumOutputs(1);
	//info->setMaximumInputs(1);
	//info->setMaximumOutputs(1);
	//info->setSource(false);
	//info->setSink(false);
	//info->setSendTransfer(false);
	//info->setReceiveTransfer(false);
	//info->insertDynamicLibFileDependence("...");
	info->setDescriptionHelp("Represents an aggregate modal model made of nodes and transitions. It can be specialized for FSM and Petri-net style execution.");
	//info->setAuthor("...");
	//info->setDate("...");
	//info->setObservation("...");
	return info;
}

ModelComponent* DefaultModalModel::LoadInstance(Model* model, PersistenceRecord *fields) {
    DefaultModalModel* newComponent = new DefaultModalModel(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newComponent;
}

ModelDataDefinition* DefaultModalModel::NewInstance(Model* model, std::string name) {
    return new DefaultModalModel(model, name);
}

//
// protected: /// virtual method that must be overriden
//

bool DefaultModalModel::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		_entryNodeName = fields->loadField("entryNodeName", DEFAULT.entryNodeName);
		_maxTransitionsPerDispatch = fields->loadField("maxTransitionsPerDispatch", DEFAULT.maxTransitionsPerDispatch);
	}
	return res;
}

void DefaultModalModel::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("entryNodeName", _entryNodeName, DEFAULT.entryNodeName, saveDefaultValues);
	fields->saveField("maxTransitionsPerDispatch", _maxTransitionsPerDispatch, DEFAULT.maxTransitionsPerDispatch, saveDefaultValues);
}

void DefaultModalModel::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	(void) inputPortNumber;
	DefaultNode* currentNode = nullptr;
	std::string currentNodeAttribute = "Entity.ModalModel." + getName() + ".CurrentNode";
	std::string lastNodeAttribute = "Entity.ModalModel." + getName() + ".LastNode";

	double index = entity->getAttributeValue(currentNodeAttribute);
	unsigned int currentIdx = static_cast<unsigned int>(index);
	if (currentIdx < _nodes->size()) {
		currentNode = _nodes->getAtRank(currentIdx);
	}
	if (currentNode == nullptr) {
		currentNode = getEntryNode();
		if (currentNode == nullptr) {
			for (DefaultNode* node : *_nodes->list()) {
				if (node->isInitialNode()) {
					currentNode = node;
					break;
				}
			}
		}
	}
	if (currentNode == nullptr && _nodes->size() > 0) {
		currentNode = _nodes->front();
	}

	unsigned int transitionsCount = 0;
	while (currentNode != nullptr && transitionsCount < _maxTransitionsPerDispatch) {
		List<DefaultNodeTransition*>* outgoing = currentNode->getTransitions();
		std::vector<DefaultNodeTransition*> enabled;
		for (DefaultNodeTransition* transition : *outgoing->list()) {
			if (transition->canFire(_parentModel, entity)) {
				enabled.push_back(transition);
			}
		}
		if (enabled.size() == 0) {
			break;
		}
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
		chosen->execute(_parentModel, entity);
		DefaultNode* nextNode = chosen->getDestination();
		if (nextNode == nullptr || nextNode == currentNode) {
			break;
		}
		currentNode = nextNode;
		transitionsCount++;
	}

	if (currentNode != nullptr) {
		for (unsigned int i = 0; i < _nodes->size(); i++) {
			if (_nodes->getAtRank(i) == currentNode) {
				entity->setAttributeValue(currentNodeAttribute, static_cast<double>(i));
				break;
			}
		}
		entity->setAttributeValue(lastNodeAttribute, static_cast<double>(currentNode->getId()));
	}

	this->_parentModel->sendEntityToComponent(entity, this->getConnectionManager()->getFrontConnection());
}


//
// protected: /// virtual methods that could be overriden by derived classes, if needed
//

/*
bool DefaultModalModel::_check(std::string& errorMessage) {
	bool resultAll = true;
	resultAll &= _someString != "";
	resultAll &= _someUint > 0;
	return resultAll;
}
*/

/*
ParserChangesInformation* DefaultModalModel::_getParserChangesInformation() {
	ParserChangesInformation* changes = new ParserChangesInformation();
	//@TODO not implemented yet
	changes->getassignments().append("");
	changes->getexpressionProductions().append("");
	changes->getexpressions().append("");
	changes->getfunctionProdutions().append("");
	changes->getassignments().append("");
	changes->getincludes().append("");
	changes->gettokens().append("");
	changes->gettypeObjs().append("");
	return changes;
}
*/

/*
void DefaultModalModel::_initBetweenReplications() {
	_someString = "Test";
	_someUint = 1;
}
*/

/*
void DefaultModalModel::_createInternalAndAttachedData() {
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
void DefaultModalModel::_addProperty(SimulationControl* property) {
}
*/
