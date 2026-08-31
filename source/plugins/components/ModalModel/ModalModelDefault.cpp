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
#include "kernel/TraitsKernel.h"
#include "../../../kernel/simulator/model/Model.h"
#include "../../../kernel/simulator/essentialPlugins/Attribute.h"
#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/PluginManager.h"
#include "kernel/statistics/SamplerDefaultImpl1.h"
#include "kernel/statistics/Sampler_if.h"
#include "plugins/data/ModalModel/DefaultNetwork.h"
#include "plugins/data/ModalModel/NetworkActivation.h"
#include <algorithm>
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
	_legacySampler = new TraitsKernel<Sampler_if>::Implementation();
}

ModalModelDefault::~ModalModelDefault() {
	delete _legacySampler;
	_legacySampler = nullptr;
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
	info->setCategory("ModalModel");
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
	_optionalEditableDataDefinitionInsert(expressionReference->getName(), expressionReference);
}

void ModalModelDefault::removeOutputExpressionReference(DefaultNodeTransition* expressionReference) {
	_optionalEditableDataDefinitionRemove(expressionReference->getName());
}

DefaultNetwork* ModalModelDefault::getNetwork() const {
	return _network;
}

void ModalModelDefault::setNetwork(DefaultNetwork* network) {
	_mandatoryAttachedAttributesClear();
	_network = network;
	_networkName = network != nullptr ? network->getName() : "";
	_syncBindingsToNetwork();
}

std::string ModalModelDefault::getNetworkName() const {
	return _networkName;
}

void ModalModelDefault::setNetworkName(const std::string& networkName) {
	_mandatoryAttachedAttributesClear();
	_networkName = networkName;
	_network = nullptr;
	_resolveNetworkReference();
	_syncBindingsToNetwork();
}

void ModalModelDefault::setInputBinding(unsigned int port, const std::string& expression) {
	if (_inputBindings.size() <= port) {
		_inputBindings.resize(port + 1, "1");
	}
	_inputBindings[port] = expression;
}

std::string ModalModelDefault::getInputBinding(unsigned int port) const {
	return port < _inputBindings.size() ? _inputBindings[port] : "";
}

void ModalModelDefault::setOutputBinding(unsigned int port, const std::string& attributeName) {
	if (_outputBindings.size() <= port) {
		_outputBindings.resize(port + 1, "");
	}
	_outputBindings[port] = attributeName;
}

std::string ModalModelDefault::getOutputBinding(unsigned int port) const {
	return port < _outputBindings.size() ? _outputBindings[port] : "";
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
		_networkName = fields->loadField("networkName", std::string(""));
		_network = nullptr;
		_resolveNetworkReference();
		_syncBindingsToNetwork();
		unsigned int inputBindings = fields->loadField("inputBindings", 0u);
		for (unsigned int i = 0; i < inputBindings; i++) {
			setInputBinding(i, fields->loadField("inputBinding" + Util::StrIndex(i), getInputBinding(i)));
		}
		unsigned int outputBindings = fields->loadField("outputBindings", 0u);
		for (unsigned int i = 0; i < outputBindings; i++) {
			setOutputBinding(i, fields->loadField("outputBinding" + Util::StrIndex(i), getOutputBinding(i)));
		}

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
	if (_network != nullptr) {
		_networkName = _network->getName();
	}
	fields->saveField("networkName", _networkName, std::string(""), saveDefaultValues);
	fields->saveField("inputBindings", static_cast<unsigned int>(_inputBindings.size()), 0u, saveDefaultValues);
	for (unsigned int bindingIndex = 0; bindingIndex < _inputBindings.size(); bindingIndex++) {
		fields->saveField("inputBinding" + Util::StrIndex(bindingIndex), _inputBindings[bindingIndex], std::string("1"), saveDefaultValues);
	}
	fields->saveField("outputBindings", static_cast<unsigned int>(_outputBindings.size()), 0u, saveDefaultValues);
	for (unsigned int bindingIndex = 0; bindingIndex < _outputBindings.size(); bindingIndex++) {
		fields->saveField("outputBinding" + Util::StrIndex(bindingIndex), _outputBindings[bindingIndex], std::string(""), saveDefaultValues);
	}
	if (_entryNode != nullptr) {
		fields->saveField("entryNode", _entryNode->getName(), "", saveDefaultValues);
	}

	fields->saveField("nodesSize", _nodes->size(), 0u, saveDefaultValues);
	unsigned int i = 0;
	for (DefaultNode* node : *_nodes->list()) {
		const std::string prefix = "node" + Util::StrIndex(i) + ".";
		auto nodeFields = std::unique_ptr<PersistenceRecord>(fields->newInstance());
		ModelDataDefinition::SaveInstance(nodeFields.get(), node);
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
	if (_network != nullptr || _networkName != "") {
		DefaultNetwork* network = _resolveNetworkReference();
		if (network == nullptr) {
			errorMessage += "ModalModelDefault \"" + getName() + "\" references unknown DefaultNetwork \"" + _networkName + "\". ";
			return false;
		}
		_syncBindingsToNetwork();
		bool resultAll = true;
		resultAll &= ModelDataDefinition::Check(network, errorMessage);
		for (unsigned int i = 0; i < network->getNumInputPorts(); i++) {
			const std::string binding = getInputBinding(i);
			if (binding == "") {
				errorMessage += "ModalModelDefault \"" + getName() + "\" input binding " + Util::StrIndex(i) + " is empty. ";
				resultAll = false;
			} else {
				resultAll &= _parentModel->checkExpression(binding, "modal input binding[" + Util::StrIndex(i) + "]", errorMessage);
				_checkCreateAttachedReferencedDataDefinition(binding);
			}
		}
		for (unsigned int i = 0; i < network->getNumOutputPorts(); i++) {
			if (getOutputBinding(i) == "") {
				errorMessage += "ModalModelDefault \"" + getName() + "\" output binding " + Util::StrIndex(i) + " is empty. ";
				resultAll = false;
			}
			if (this->getConnectionManager()->getConnectionAtPort(i) == nullptr) {
				errorMessage += "ModalModelDefault \"" + getName() + "\" output port " + Util::StrIndex(i) + " has no outgoing connection. ";
				resultAll = false;
			}
		}
		return resultAll;
	}
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
	_resetLegacySampler();
}

void ModalModelDefault::_createAttachedAttributes() {
	_mandatoryAttachedAttributesClear();
	_syncBindingsToNetwork();
	std::vector<std::string> outputAttributes;
	for (const std::string& outputBinding : _outputBindings) {
		if (outputBinding != "") {
			outputAttributes.push_back(outputBinding);
		}
	}
	if (!outputAttributes.empty()) {
		_attachedAttributesInsert(outputAttributes);
	}
	if (_network != nullptr || _networkName != "") {
		return;
	}

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
	if (_network != nullptr || _networkName != "") {
		if (_dispatchNetworkActivation(entity, inputPortNumber)) {
			return;
		}
	}
	_dispatchLegacyNodeModel(entity, inputPortNumber);
}

DefaultNetwork* ModalModelDefault::_resolveNetworkReference() {
	if (_network != nullptr || _networkName == "") {
		return _network;
	}
	for (const std::string& className : _parentModel->getDataManager()->getDataDefinitionClassnames()) {
		List<ModelDataDefinition*>* definitions = _parentModel->getDataManager()->getDataDefinitionList(className);
		for (ModelDataDefinition* definition : *definitions->list()) {
			DefaultNetwork* candidate = dynamic_cast<DefaultNetwork*>(definition);
			if (candidate != nullptr && candidate->getName() == _networkName) {
				_network = candidate;
				return _network;
			}
		}
	}
	return nullptr;
}

void ModalModelDefault::_syncBindingsToNetwork() {
	DefaultNetwork* network = _resolveNetworkReference();
	if (network == nullptr) {
		return;
	}
	_connections->setMinInputConnections(network->getNumInputPorts());
	_connections->setMaxInputConnections(network->getNumInputPorts());
	_connections->setMinOutputConnections(network->getNumOutputPorts());
	_connections->setMaxOutputConnections(network->getNumOutputPorts());
	if (_inputBindings.size() < network->getNumInputPorts()) {
		_inputBindings.resize(network->getNumInputPorts(), "1");
	}
	if (_inputBindings.size() > network->getNumInputPorts()) {
		_inputBindings.resize(network->getNumInputPorts());
	}
	if (_outputBindings.size() < network->getNumOutputPorts()) {
		unsigned int oldSize = static_cast<unsigned int>(_outputBindings.size());
		_outputBindings.resize(network->getNumOutputPorts(), "");
		for (unsigned int i = oldSize; i < network->getNumOutputPorts(); i++) {
			_outputBindings[i] = network->getOutputPortName(i);
		}
	}
	if (_outputBindings.size() > network->getNumOutputPorts()) {
		_outputBindings.resize(network->getNumOutputPorts());
	}
}

bool ModalModelDefault::_dispatchNetworkActivation(Entity* entity, unsigned int inputPortNumber) {
	DefaultNetwork* network = _resolveNetworkReference();
	if (network == nullptr) {
		traceError("ModalModelDefault \"" + getName() + "\" has no resolvable DefaultNetwork.");
		return false;
	}
	_syncBindingsToNetwork();
	if (inputPortNumber >= network->getNumInputPorts()) {
		traceError("ModalModelDefault \"" + getName() + "\" received input port " + Util::StrIndex(inputPortNumber) +
		           " but attached network \"" + network->getName() + "\" has only " + Util::StrIndex(network->getNumInputPorts()) + " inputs.");
		_parentModel->removeEntity(entity);
		return true;
	}

	NetworkActivationFrame frame(network->getNumInputPorts());
	const std::string inputBinding = getInputBinding(inputPortNumber);
	frame.setPresent(inputPortNumber, _parentModel->parseExpression(inputBinding));
	NetworkActivationResult result = network->activate(frame);
	_routeNetworkOutputs(entity, result);
	return true;
}

Entity* ModalModelDefault::_cloneEntity(Entity* entity) {
	if (entity == nullptr) {
		return nullptr;
	}
	std::string cloneBaseName = entity->getEntityType() != nullptr ? entity->getEntityType()->getName() : entity->getName();
	Entity* clone = _parentModel->createEntity(cloneBaseName + "_%", true);
	clone->setEntityType(entity->getEntityType());
	for (ModelDataDefinition* attributeDefinition : *_parentModel->getDataManager()->getDataDefinitionList(Util::TypeOf<Attribute>())->list()) {
		const std::string attributeName = attributeDefinition->getName();
		clone->setAttributeValue(attributeName, entity->getAttributeValue(attributeName));
	}
	return clone;
}

void ModalModelDefault::_writeOutputBinding(Entity* entity, unsigned int outputPort, double value) {
	const std::string attributeName = getOutputBinding(outputPort);
	if (attributeName != "") {
		entity->setAttributeValue(attributeName, value, "", true);
	}
}

void ModalModelDefault::_routeNetworkOutputs(Entity* entity, const NetworkActivationResult& result) {
	if (result.countPresent() == 0) {
		_parentModel->removeEntity(entity);
		return;
	}

	std::vector<unsigned int> presentOutputs;
	for (unsigned int outputPort = 0; outputPort < result.size(); outputPort++) {
		if (result.isPresent(outputPort)) {
			presentOutputs.push_back(outputPort);
		}
	}

	for (unsigned int i = 0; i < presentOutputs.size(); i++) {
		const unsigned int outputPort = presentOutputs[i];
		Entity* outgoing = i + 1 == presentOutputs.size() ? entity : _cloneEntity(entity);
		_writeOutputBinding(outgoing, outputPort, result.getValue(outputPort));
		Connection* connection = this->getConnectionManager()->getConnectionAtPort(outputPort);
		if (connection == nullptr) {
			traceError("ModalModelDefault \"" + getName() + "\" has no connection for present output port " + Util::StrIndex(outputPort) + ".");
			_parentModel->removeEntity(outgoing);
		} else {
			_parentModel->sendEntityToComponent(outgoing, connection);
		}
	}
}

void ModalModelDefault::_dispatchLegacyNodeModel(Entity* entity, unsigned int inputPortNumber) {
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
			DefaultNodeTransition* chosen = _chooseLegacyTransition(enabled);
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

void ModalModelDefault::_resetLegacySampler() {
	SamplerDefaultImpl1* defaultSampler = dynamic_cast<SamplerDefaultImpl1*>(_legacySampler);
	if (defaultSampler != nullptr) {
		defaultSampler->reset();
	}
}

DefaultNodeTransition* ModalModelDefault::_chooseLegacyTransition(const std::vector<DefaultNodeTransition*>& enabled) {
	if (enabled.empty()) {
		return nullptr;
	}
	std::vector<DefaultNodeTransition*> ordered = enabled;
	std::sort(ordered.begin(), ordered.end(), [](DefaultNodeTransition* a, DefaultNodeTransition* b) {
		return a->getPriority() < b->getPriority();
	});
	DefaultNodeTransition* chosen = ordered.front();
	if (chosen->getTransitionKind() != DefaultNodeTransition::TransitionKind::PROBABILISTIC) {
		return chosen;
	}

	double probabilitySum = 0.0;
	for (DefaultNodeTransition* option : ordered) {
		if (option != nullptr) {
			probabilitySum += option->getProbability();
		}
	}
	if (probabilitySum <= 0.0) {
		return chosen;
	}

	const double sample = _legacySampler->random() * probabilitySum;
	double accum = 0.0;
	for (DefaultNodeTransition* option : ordered) {
		if (option == nullptr) {
			continue;
		}
		accum += option->getProbability();
		if (sample <= accum) {
			return option;
		}
	}
	return chosen;
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
void ModalModelDefault::_addSimulationControl(SimulationControl* property) {
}
*/

// void ModalModelDefault::_createInternalStatisticReporters() { }

// void ModalModelDefault::_createEditableDataDefinitions() { }
