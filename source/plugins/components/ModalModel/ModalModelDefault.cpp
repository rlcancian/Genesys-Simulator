/*
 * File:   ModalModelDefault.cpp
 * Author: Prof. Rafael Luiz Cancian, Dr. Eng.
 *
 * Created on 01 de Julho de 2025, 14:26
 */

#include "plugins/components/ModalModel/ModalModelDefault.h"
#include "../../../kernel/simulator/model/Model.h"
#include "../../../kernel/simulator/essentialPlugins/Attribute.h"
#include "plugins/data/ModalModel/DefaultNetwork.h"
#include "plugins/data/ModalModel/NetworkActivation.h"

#ifdef PLUGINCONNECT_DYNAMIC
extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &ModalModelDefault::GetPluginInformation;
}
#endif

ModalModelDefault::ModalModelDefault(Model* model, std::string name)
	: ModelComponent(model, Util::TypeOf<ModalModelDefault>(), name) {
}

void ModalModelDefault::setMaxTransitionsPerDispatch(unsigned int maxTransitionsPerDispatch) {
	_maxTransitionsPerDispatch = maxTransitionsPerDispatch;
}

unsigned int ModalModelDefault::getMaxTransitionsPerDispatch() const {
	return _maxTransitionsPerDispatch;
}

std::string ModalModelDefault::show() {
	return ModelComponent::show() +
	       ", network=\"" + (_network != nullptr ? _network->getName() : _networkName) + "\"";
}

PluginInformation* ModalModelDefault::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<ModalModelDefault>(), &ModalModelDefault::LoadInstance,
	                                                &ModalModelDefault::NewInstance);
	info->setCategory("ModalModel");
	info->setMaximumOutputs(2);
	info->setDescriptionHelp(
		"Process adapter that activates an attached DefaultNetwork and routes network outputs back into the process model.");
	return info;
}

ModelComponent* ModalModelDefault::LoadInstance(Model* model, PersistenceRecord* fields) {
	ModalModelDefault* newComponent = new ModalModelDefault(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {
		(void)e;
	}
	return newComponent;
}

ModelDataDefinition* ModalModelDefault::NewInstance(Model* model, std::string name) {
	return new ModalModelDefault(model, name);
}

void ModalModelDefault::addOutputExpressionReference(ModelDataDefinition* expressionReference) {
	if (expressionReference != nullptr) {
		_optionalEditableDataDefinitionInsert(expressionReference->getName(), expressionReference);
	}
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
}

bool ModalModelDefault::_check(std::string& errorMessage) {
	DefaultNetwork* network = _resolveNetworkReference();
	if (network == nullptr) {
		errorMessage += "ModalModelDefault \"" + getName() + "\" requires an attached DefaultNetwork";
		if (_networkName != "") {
			errorMessage += " (unknown reference \"" + _networkName + "\")";
		}
		errorMessage += ". ";
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

void ModalModelDefault::_initBetweenReplications() {
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
	if (!_dispatchNetworkActivation(entity, inputPortNumber)) {
		traceError("ModalModelDefault \"" + getName() + "\" could not activate its DefaultNetwork.");
		_parentModel->removeEntity(entity);
	}
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
