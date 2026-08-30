/*
 * File:   DefaultNetwork.cpp
 * Author: Prof. Rafael Luiz Cancian, Dr. Eng.
 *
 * Created on 30 de Agosto de 2026
 */

#include "DefaultNetwork.h"
#include "../../../kernel/simulator/model/Model.h"
#include "../../../kernel/simulator/essentialPlugins/Counter.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &DefaultNetwork::GetPluginInformation;
}
#endif

ModelDataDefinition* DefaultNetwork::NewInstance(Model* model, std::string name) {
	return new DefaultNetwork(model, name);
}

DefaultNetwork::DefaultNetwork(Model* model, std::string name, std::string dataDefinitionTypename)
	: ModelDataDefinition(model, dataDefinitionTypename, name) {
}

DefaultNetwork::~DefaultNetwork() {
	delete _inputPortNames;
	delete _outputPortNames;
}

std::string DefaultNetwork::show() {
	return ModelDataDefinition::show() +
			", inputs=" + std::to_string(_inputPortNames->size()) +
			", outputs=" + std::to_string(_outputPortNames->size()) +
			", activationCount=" + std::to_string(getActivationCount());
}

unsigned int DefaultNetwork::addInputPort(const std::string& portName) {
	int existing = getInputPortIndex(portName);
	if (existing >= 0) {
		return static_cast<unsigned int>(existing);
	}
	_inputPortNames->insert(portName);
	return _inputPortNames->size() - 1;
}

unsigned int DefaultNetwork::addOutputPort(const std::string& portName) {
	int existing = getOutputPortIndex(portName);
	if (existing >= 0) {
		return static_cast<unsigned int>(existing);
	}
	_outputPortNames->insert(portName);
	return _outputPortNames->size() - 1;
}

unsigned int DefaultNetwork::getNumInputPorts() const {
	return _inputPortNames->size();
}

unsigned int DefaultNetwork::getNumOutputPorts() const {
	return _outputPortNames->size();
}

std::string DefaultNetwork::getInputPortName(unsigned int index) const {
	return index < _inputPortNames->size() ? _inputPortNames->getAtRank(index) : "";
}

std::string DefaultNetwork::getOutputPortName(unsigned int index) const {
	return index < _outputPortNames->size() ? _outputPortNames->getAtRank(index) : "";
}

int DefaultNetwork::getInputPortIndex(const std::string& portName) const {
	unsigned int i = 0;
	for (const std::string& existingName : *_inputPortNames->list()) {
		if (existingName == portName) {
			return static_cast<int>(i);
		}
		i++;
	}
	return -1;
}

int DefaultNetwork::getOutputPortIndex(const std::string& portName) const {
	unsigned int i = 0;
	for (const std::string& existingName : *_outputPortNames->list()) {
		if (existingName == portName) {
			return static_cast<int>(i);
		}
		i++;
	}
	return -1;
}

double DefaultNetwork::getActivationCount() const {
	return _activationCounter != nullptr ? _activationCounter->getCountValue() : 0.0;
}

NetworkActivationResult DefaultNetwork::activate(const NetworkActivationFrame& frame) {
	if (_reportStatistics) {
		if (_activationCounter == nullptr) {
			_createInternalStatisticReporters();
		}
		if (_activationCounter != nullptr) {
			_activationCounter->incCountValue();
		}
	}
	return _activate(frame);
}

NetworkActivationResult DefaultNetwork::_activate(const NetworkActivationFrame& /*frame*/) {
	// Base DefaultNetwork has no formalism semantics: every declared output stays absent.
	return NetworkActivationResult(getNumOutputPorts());
}

PluginInformation* DefaultNetwork::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<DefaultNetwork>(), &DefaultNetwork::LoadInstance, &DefaultNetwork::NewInstance);
	info->setCategory("ModalModel");
	info->setDescriptionHelp("Base network model of computation (formal input/output ports, activation counter). Use a specialization (EFSMNetwork, MarkovChainNetwork, ColoredPetriNetNetwork) for actual formalism semantics.");
	return info;
}

ModelDataDefinition* DefaultNetwork::LoadInstance(Model* model, PersistenceRecord *fields) {
	DefaultNetwork* newElement = new DefaultNetwork(model);
	try {
		newElement->_loadInstance(fields);
	} catch (const std::exception& e) {
		newElement->traceError("Failed to load DefaultNetwork instance: " + std::string(e.what()));
	}
	return newElement;
}

bool DefaultNetwork::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelDataDefinition::_loadInstance(fields);
	if (res) {
		unsigned int numInputs = fields->loadField("inputPorts", 0u);
		for (unsigned int i = 0; i < numInputs; i++) {
			_inputPortNames->insert(fields->loadField("inputPort" + Util::StrIndex(i), std::string("")));
		}
		unsigned int numOutputs = fields->loadField("outputPorts", 0u);
		for (unsigned int i = 0; i < numOutputs; i++) {
			_outputPortNames->insert(fields->loadField("outputPort" + Util::StrIndex(i), std::string("")));
		}
	}
	return res;
}

void DefaultNetwork::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelDataDefinition::_saveInstance(fields, saveDefaultValues);
	fields->saveField("inputPorts", _inputPortNames->size(), 0u, saveDefaultValues);
	unsigned int i = 0;
	for (const std::string& portName : *_inputPortNames->list()) {
		fields->saveField("inputPort" + Util::StrIndex(i), portName, std::string(""), saveDefaultValues);
		i++;
	}
	fields->saveField("outputPorts", _outputPortNames->size(), 0u, saveDefaultValues);
	i = 0;
	for (const std::string& portName : *_outputPortNames->list()) {
		fields->saveField("outputPort" + Util::StrIndex(i), portName, std::string(""), saveDefaultValues);
		i++;
	}
}

bool DefaultNetwork::_check(std::string& errorMessage) {
	bool resultAll = true;
	unsigned int index = 0;
	for (const std::string& portName : *_inputPortNames->list()) {
		if (portName.empty()) {
			errorMessage += "DefaultNetwork \"" + getName() + "\" input port " + Util::StrIndex(index) + " has an empty name. ";
			resultAll = false;
		}
		index++;
	}
	index = 0;
	for (const std::string& portName : *_outputPortNames->list()) {
		if (portName.empty()) {
			errorMessage += "DefaultNetwork \"" + getName() + "\" output port " + Util::StrIndex(index) + " has an empty name. ";
			resultAll = false;
		}
		index++;
	}
	return resultAll;
}

void DefaultNetwork::_initBetweenReplications() {
	// Counter is internal data; the base class implementation already resets every
	// internal-data child (including _activationCounter) via ModelDataDefinition::_initBetweenReplications().
	ModelDataDefinition::_initBetweenReplications();
}

void DefaultNetwork::_createInternalStatisticReporters() {
	if (_reportStatistics) {
		if (_activationCounter == nullptr) {
			_activationCounter = new Counter(_parentModel, getName() + "." + "ActivationCount", this);
		}
		_mandatoryNonEditableDataDefinitionInsert("ActivationCount", _activationCounter);
	} else {
		_mandatoryNonEditableDataDefinitionRemove("ActivationCount");
		_activationCounter = nullptr;
	}
}
