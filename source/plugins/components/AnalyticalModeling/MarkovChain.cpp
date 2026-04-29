/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   MarkovChain.cpp
 * Author: rlcancian
 *
 * Created on 24 de Outubro de 2019, 18:26
 */

#include "plugins/components/AnalyticalModeling/MarkovChain.h"

#include "kernel/TraitsKernel.h"
#include "kernel/simulator/Attribute.h"
#include "kernel/simulator/Model.h"
#include "kernel/simulator/SimulationControlAndResponse.h"
#include "kernel/simulator/Simulator.h"
#include "plugins/data/DiscreteProcessing/Variable.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace {

bool isMarkovChainSupportedDefinition(const ModelDataDefinition* definition) {
	return definition == nullptr ||
	       definition->getClassname() == Util::TypeOf<Variable>() ||
	       definition->getClassname() == Util::TypeOf<Attribute>();
}

std::string formatDefinitionReference(const ModelDataDefinition* definition) {
	if (definition == nullptr) {
		return "";
	}
	return definition->getClassname() + ":" + definition->getName();
}

std::pair<std::string, std::string> parseDefinitionReference(const std::string& value) {
	const std::size_t separator = value.find(':');
	if (separator == std::string::npos) {
		return {"", value};
	}
	return {value.substr(0, separator), value.substr(separator + 1)};
}

ModelDataDefinition* resolveDefinitionReference(Model* model, const std::string& value) {
	if (model == nullptr || value.empty()) {
		return nullptr;
	}
	const std::pair<std::string, std::string> parsed = parseDefinitionReference(value);
	if (!parsed.first.empty()) {
		return model->getDataManager()->getDataDefinition(parsed.first, parsed.second);
	}
	ModelDataDefinition* definition = model->getDataManager()->getDataDefinition(Util::TypeOf<Variable>(), parsed.second);
	if (definition != nullptr) {
		return definition;
	}
	return model->getDataManager()->getDataDefinition(Util::TypeOf<Attribute>(), parsed.second);
}

class MarkovChainDataDefinitionReferenceControl : public SimulationControl {
public:
	MarkovChainDataDefinitionReferenceControl(Model* model,
	                                          GetterGeneric<ModelDataDefinition*> getter,
	                                          SetterGeneric<ModelDataDefinition*> setter,
	                                          const std::string& className,
	                                          const std::string& elementName,
	                                          const std::string& propertyName,
	                                          const std::string& whatsThis = "")
	    : SimulationControl(className, elementName, propertyName, whatsThis, false, true, false),
	      _model(model),
	      _getter(std::move(getter)),
	      _setter(std::move(setter)) {
		_readonly = _setter == nullptr;
		_propertyType = Util::TypeOf<ModelDataDefinition>();
	}

	std::string getValue() const override {
		return formatDefinitionReference(_getter != nullptr ? _getter() : nullptr);
	}

	void setValue(std::string value, bool remove = false) override {
		_ensureWritable("set value of");
		if (_setter == nullptr) {
			throw std::logic_error("MarkovChainDataDefinitionReferenceControl setter is not defined");
		}
		if (remove || value.empty()) {
			_setter(nullptr);
			return;
		}
		ModelDataDefinition* definition = resolveDefinitionReference(_model, value);
		if (!isMarkovChainSupportedDefinition(definition)) {
			throw std::invalid_argument("MarkovChain supports only Variable or Attribute references.");
		}
		if (definition == nullptr) {
			throw std::invalid_argument("Referenced Variable or Attribute was not found.");
		}
		_setter(definition);
	}

	bool isModelDataDefinitionReference() const override { return true; }
	ModelDataDefinition* getReferencedModelDataDefinition() const override {
		return _getter != nullptr ? _getter() : nullptr;
	}
	bool supportsInlineExpansion() const override { return true; }
	bool supportsExistingObjectSelection() const override { return true; }
	bool supportsObjectCreation() const override { return false; }
	bool isInlineObjectProperty() const override { return false; }
	bool hasObjectInstance() const override { return getReferencedModelDataDefinition() != nullptr; }
	bool ensureObjectInstance() override { return hasObjectInstance(); }
	List<std::string>* getStrValues() override {
		List<std::string>* options = new List<std::string>();
		for (const std::string& typeName : {Util::TypeOf<Variable>(), Util::TypeOf<Attribute>()}) {
			List<ModelDataDefinition*>* definitions = _model->getDataManager()->getDataDefinitionList(typeName);
			if (definitions == nullptr) {
				continue;
			}
			for (ModelDataDefinition* definition : *definitions->list()) {
				if (definition != nullptr) {
					options->insert(formatDefinitionReference(definition));
				}
			}
		}
		return options;
	}
	List<SimulationControl*>* getChildSimulationControls(int index = 0) override {
		(void)index;
		ModelDataDefinition* definition = getReferencedModelDataDefinition();
		return definition != nullptr ? definition->getSimulationControls() : nullptr;
	}

private:
	Model* _model = nullptr;
	GetterGeneric<ModelDataDefinition*> _getter;
	SetterGeneric<ModelDataDefinition*> _setter;
};

double readDefinitionValue(ModelDataDefinition* definition, Entity* entity, const std::string& index = "") {
	if (definition == nullptr) {
		return 0.0;
	}
	if (definition->getClassname() == Util::TypeOf<Variable>()) {
		return static_cast<Variable*>(definition)->getValue(index);
	}
	if (definition->getClassname() == Util::TypeOf<Attribute>()) {
		if (entity != nullptr) {
			return entity->getAttributeValue(definition->getName(), index);
		}
		return static_cast<Attribute*>(definition)->getInitialValue(index);
	}
	throw std::invalid_argument("Unsupported MarkovChain data definition type.");
}

void writeDefinitionValue(ModelDataDefinition* definition, Entity* entity, double value, const std::string& index = "") {
	if (definition == nullptr) {
		return;
	}
	if (definition->getClassname() == Util::TypeOf<Variable>()) {
		static_cast<Variable*>(definition)->setValue(value, index);
		return;
	}
	if (definition->getClassname() == Util::TypeOf<Attribute>()) {
		if (entity != nullptr) {
			entity->setAttributeValue(definition->getName(), value, index, true);
		} else {
			static_cast<Attribute*>(definition)->setInitialValue(value, index);
		}
		return;
	}
	throw std::invalid_argument("Unsupported MarkovChain data definition type.");
}

} // namespace

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &MarkovChain::GetPluginInformation;
}
#endif

ModelDataDefinition* MarkovChain::NewInstance(Model* model, std::string name) {
	return new MarkovChain(model, name);
}

MarkovChain::MarkovChain(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<MarkovChain>(), name) {
	_sampler = new TraitsKernel<Sampler_if>::Implementation();

	MarkovChainDataDefinitionReferenceControl* propTransitionMatrix = new MarkovChainDataDefinitionReferenceControl(
			_parentModel,
			std::bind(&MarkovChain::getTransitionMatrix, this),
			std::bind(&MarkovChain::setTransitionProbabilityMatrix, this, std::placeholders::_1),
			Util::TypeOf<MarkovChain>(), getName(), "TransitionMatrix", "");
	MarkovChainDataDefinitionReferenceControl* propCurrentState = new MarkovChainDataDefinitionReferenceControl(
			_parentModel,
			std::bind(&MarkovChain::getCurrentState, this),
			std::bind(&MarkovChain::setCurrentState, this, std::placeholders::_1),
			Util::TypeOf<MarkovChain>(), getName(), "CurrentState", "");

	_parentModel->getControls()->insert(propTransitionMatrix);
	_parentModel->getControls()->insert(propCurrentState);
	_addSimulationControl(propTransitionMatrix);
	_addSimulationControl(propCurrentState);
}

MarkovChain::~MarkovChain() {
	delete _sampler;
	_sampler = nullptr;
}

std::string MarkovChain::show() {
	return ModelComponent::show() + "";
}

ModelComponent* MarkovChain::LoadInstance(Model* model, PersistenceRecord *fields) {
	MarkovChain* newComponent = new MarkovChain(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {
		(void)e;
	}
	return newComponent;
}

void MarkovChain::setTransitionProbabilityMatrix(ModelDataDefinition* transitionMatrix) {
	if (!isMarkovChainSupportedDefinition(transitionMatrix)) {
		throw std::invalid_argument("TransitionMatrix must reference a Variable or an Attribute.");
	}
	_transitionProbMatrix = transitionMatrix;
}

ModelDataDefinition* MarkovChain::getTransitionMatrix() const {
	return _transitionProbMatrix;
}

void MarkovChain::setCurrentState(ModelDataDefinition* currentState) {
	if (!isMarkovChainSupportedDefinition(currentState)) {
		throw std::invalid_argument("CurrentState must reference a Variable or an Attribute.");
	}
	_currentState = currentState;
}

ModelDataDefinition* MarkovChain::getCurrentState() const {
	return _currentState;
}

void MarkovChain::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	(void)inputPortNumber;
	if (entity == nullptr) {
		traceSimulation(this, "MarkovChain received a null entity; no transition was sampled.", TraceManager::Level::L7_internal);
		return;
	}
	if (_transitionProbMatrix == nullptr || _currentState == nullptr) {
		traceSimulation(this, "MarkovChain is missing TransitionMatrix or CurrentState; entity was forwarded unchanged.", TraceManager::Level::L7_internal);
		_parentModel->sendEntityToComponent(entity, this->getConnectionManager()->getFrontConnection());
		return;
	}
	if (_currentState->getClassname() == Util::TypeOf<Attribute>() &&
	    _parentModel->getDataManager()->getDataDefinition(Util::TypeOf<Attribute>(), _currentState->getName()) == nullptr) {
		new Attribute(_parentModel, _currentState->getName());
	}

	const unsigned int currentState = static_cast<unsigned int>(std::max(0.0, _readStateValue(entity)));
	const unsigned int nextState = _drawNextState(entity, currentState);
	_writeStateValue(entity, nextState);

	traceSimulation(this,
	                "MarkovChain sampled entity " + entity->getName() + " transition " + std::to_string(currentState) + " -> " +
	                    std::to_string(nextState) + " using " + formatDefinitionReference(_currentState),
	                TraceManager::Level::L7_internal);
	_parentModel->sendEntityToComponent(entity, this->getConnectionManager()->getFrontConnection());
}

bool MarkovChain::_loadInstance(PersistenceRecord *fields) {
	const bool result = ModelComponent::_loadInstance(fields);
	if (result) {
		const std::string transitionReference = fields->loadField("transitionMatrixRef",
		                                                          fields->loadField("transitionMatrix", ""));
		const std::string currentStateReference = fields->loadField("currentStateRef",
		                                                            fields->loadField("currentState", ""));
		setTransitionProbabilityMatrix(resolveDefinitionReference(_parentModel, transitionReference));
		setCurrentState(resolveDefinitionReference(_parentModel, currentStateReference));
	}
	return result;
}

void MarkovChain::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	if (_transitionProbMatrix != nullptr) {
		fields->saveField("transitionMatrixRef", formatDefinitionReference(_transitionProbMatrix), "", saveDefaultValues);
		fields->saveField("transitionMatrix", _transitionProbMatrix->getName(), "", saveDefaultValues);
	}
	if (_currentState != nullptr) {
		fields->saveField("currentStateRef", formatDefinitionReference(_currentState), "", saveDefaultValues);
		fields->saveField("currentState", _currentState->getName(), "", saveDefaultValues);
	}
}

void MarkovChain::_initBetweenReplications() {
}

bool MarkovChain::_check(std::string& errorMessage) {
	bool resultAll = true;
	if (_transitionProbMatrix == nullptr) {
		errorMessage += "MarkovChain requires a transition probability matrix. ";
		resultAll = false;
	}
	if (_currentState == nullptr) {
		errorMessage += "MarkovChain requires a current state reference. ";
		resultAll = false;
	}
	if (_transitionProbMatrix != nullptr && _stateCount() == 0) {
		errorMessage += "MarkovChain transition matrix must expose at least one state. ";
		resultAll = false;
	}
	return resultAll;
}

void MarkovChain::_createInternalAndAttachedData() {
	_attachedDataClear();
	if (_transitionProbMatrix != nullptr) {
		_attachedDataInsert(_transitionProbMatrix->getName(), _transitionProbMatrix);
	}
	if (_currentState != nullptr) {
		_attachedDataInsert(_currentState->getName(), _currentState);
	}
}

PluginInformation* MarkovChain::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<MarkovChain>(), &MarkovChain::LoadInstance, &MarkovChain::NewInstance);
	info->setCategory("AnalyticalModeling");
	return info;
}

void MarkovChain::_createReportStatisticsDataDefinitions() {
}

void MarkovChain::_createEditableDataDefinitions() {
}

void MarkovChain::_createOthersDataDefinitions() {
}

unsigned int MarkovChain::_drawNextState(Entity* entity, unsigned int currentState) {
	const unsigned int size = _stateCount();
	if (_transitionProbMatrix == nullptr || size == 0) {
		return currentState;
	}
	if (currentState >= size) {
		traceSimulation(this,
		                "MarkovChain current state " + std::to_string(currentState) + " is outside the transition matrix; using state 0.",
		                TraceManager::Level::L7_internal);
		currentState = 0;
	}

	const double rnd = _sampler->random();
	double sum = 0.0;
	for (unsigned int nextState = 0; nextState < size; ++nextState) {
		const std::string index = std::to_string(currentState) + "," + std::to_string(nextState);
		const double probability = _readTransitionProbability(entity, currentState, nextState);
		sum += probability;
		traceSimulation(this,
		                "MarkovChain transition probability p[" + index + "]=" + std::to_string(probability) +
		                    ", cumulative=" + std::to_string(sum),
		                TraceManager::Level::L9_mostDetailed);
		if (sum >= rnd) {
			return nextState;
		}
	}
	return size - 1;
}

unsigned int MarkovChain::_stateCount() const {
	if (_transitionProbMatrix == nullptr) {
		return 0;
	}
	std::list<unsigned int>* dimensions = nullptr;
	if (_transitionProbMatrix->getClassname() == Util::TypeOf<Variable>()) {
		dimensions = static_cast<Variable*>(_transitionProbMatrix)->getDimensionSizes();
	} else if (_transitionProbMatrix->getClassname() == Util::TypeOf<Attribute>()) {
		dimensions = static_cast<Attribute*>(_transitionProbMatrix)->getDimensionSizes();
	}
	if (dimensions == nullptr || dimensions->empty()) {
		return 0;
	}
	return dimensions->front();
}

double MarkovChain::_readStateValue(Entity* entity) const {
	const double value = readDefinitionValue(_currentState, entity);
	const_cast<MarkovChain*>(this)->traceSimulation(
			const_cast<MarkovChain*>(this),
			"MarkovChain read current state " + std::to_string(value) + " from " +
			    formatDefinitionReference(_currentState),
			TraceManager::Level::L8_detailed);
	return value;
}

void MarkovChain::_writeStateValue(Entity* entity, double value) {
	writeDefinitionValue(_currentState, entity, value);
	traceSimulation(this,
	                "MarkovChain wrote current state " + std::to_string(value) + " to " +
	                    formatDefinitionReference(_currentState),
	                TraceManager::Level::L8_detailed);
}

double MarkovChain::_readTransitionProbability(Entity* entity, unsigned int fromState, unsigned int toState) const {
	return readDefinitionValue(_transitionProbMatrix, entity,
	                           std::to_string(fromState) + "," + std::to_string(toState));
}
