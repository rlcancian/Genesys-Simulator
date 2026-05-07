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
#include "../../data/Logic/Variable.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iterator>
#include <stdexcept>
#include <utility>
#include <vector>

namespace {

bool isMarkovChainSupportedDefinition(const ModelDataDefinition* definition) {
	return definition == nullptr ||
	       definition->getClassname() == Util::TypeOf<Variable>() ||
	       definition->getClassname() == Util::TypeOf<Attribute>();
}

const SparseValueStore* getDefinitionValueStore(const ModelDataDefinition* definition) {
	if (definition == nullptr) {
		return nullptr;
	}
	if (definition->getClassname() == Util::TypeOf<Variable>()) {
		const Variable* variable = static_cast<const Variable*>(definition);
		const SparseValueStore* runtimeStore = const_cast<Variable*>(variable)->getValueStore();
		if (runtimeStore != nullptr && !runtimeStore->values()->empty()) {
			return runtimeStore;
		}
		const SparseValueStore* initialStore = const_cast<Variable*>(variable)->getInitialValueStore();
		if (initialStore != nullptr && !initialStore->values()->empty()) {
			return initialStore;
		}
		if (runtimeStore != nullptr && !runtimeStore->dimensionSizes()->empty()) {
			return runtimeStore;
		}
		return initialStore;
	}
	if (definition->getClassname() == Util::TypeOf<Attribute>()) {
		return const_cast<Attribute*>(static_cast<const Attribute*>(definition))->getInitialValueStore();
	}
	return nullptr;
}

const std::list<unsigned int>* getDefinitionDimensionSizes(const ModelDataDefinition* definition) {
	const SparseValueStore* store = getDefinitionValueStore(definition);
	return store != nullptr ? store->dimensionSizes() : nullptr;
}

double readDefinitionValue(const ModelDataDefinition* definition, const std::string& index) {
	const SparseValueStore* store = getDefinitionValueStore(definition);
	if (store == nullptr) {
		throw std::invalid_argument("Unsupported MarkovChain data definition type.");
	}
	return store->value(index);
}

bool validateTransitionMatrixDefinition(const ModelDataDefinition* definition, std::string& errorMessage) {
	const std::list<unsigned int>* dimensions = getDefinitionDimensionSizes(definition);
	if (dimensions == nullptr || dimensions->size() != 2u) {
		errorMessage += "MarkovChain transition probability matrix must be two-dimensional. ";
		return false;
	}

	const auto dimIt = dimensions->begin();
	const unsigned int rowCount = *dimIt;
	const unsigned int columnCount = *std::next(dimIt);
	if (rowCount == 0u || columnCount == 0u) {
		errorMessage += "MarkovChain transition probability matrix must not be empty. ";
		return false;
	}
	if (rowCount != columnCount) {
		errorMessage += "MarkovChain transition probability matrix must be square. ";
		return false;
	}

	constexpr double probabilityTolerance = 1e-9;
	for (unsigned int row = 0; row < rowCount; ++row) {
		double rowSum = 0.0;
		for (unsigned int column = 0; column < columnCount; ++column) {
			const std::string index = std::to_string(row) + "," + std::to_string(column);
			const double probability = readDefinitionValue(definition, index);
			if (!std::isfinite(probability) || probability < -probabilityTolerance || probability > 1.0 + probabilityTolerance) {
				errorMessage += "MarkovChain transition probability p[" + index + "] must be between 0 and 1. ";
				return false;
			}
			rowSum += probability;
		}
		if (std::fabs(rowSum - 1.0) > probabilityTolerance) {
			errorMessage += "MarkovChain transition probability row " + std::to_string(row) +
			                " must sum to 1.0. ";
			return false;
		}
	}
	return true;
}

bool validateInitialDistributionDefinition(const ModelDataDefinition* initialDistribution, unsigned int stateCount, std::string& errorMessage) {
	const std::list<unsigned int>* dimensions = getDefinitionDimensionSizes(initialDistribution);
	if (dimensions == nullptr) {
		errorMessage += "MarkovChain initial distribution reference is invalid. ";
		return false;
	}
	if (dimensions->size() > 1u) {
		errorMessage += "MarkovChain initial distribution must be scalar or one-dimensional. ";
		return false;
	}
	const unsigned int distributionSize = dimensions->empty() ? 1u : dimensions->front();
	if (distributionSize != stateCount) {
		errorMessage += "MarkovChain initial distribution vector size must match the transition matrix size. ";
		return false;
	}
	constexpr double probabilityTolerance = 1e-9;
	double probabilitySum = 0.0;
	for (unsigned int state = 0; state < stateCount; ++state) {
		const std::string index = dimensions->empty() ? "" : std::to_string(state);
		const double probability = readDefinitionValue(initialDistribution, index);
		if (!std::isfinite(probability) || probability < -probabilityTolerance || probability > 1.0 + probabilityTolerance) {
			errorMessage += "MarkovChain initial distribution p[" + std::to_string(state) + "] must be between 0 and 1. ";
			return false;
		}
		probabilitySum += probability;
	}
	if (std::fabs(probabilitySum - 1.0) > probabilityTolerance) {
		errorMessage += "MarkovChain initial distribution must sum to 1.0. ";
		return false;
	}
	return true;
}

bool validateCurrentStateDefinition(const ModelDataDefinition* currentState, std::string& errorMessage) {
	const std::list<unsigned int>* dimensions = getDefinitionDimensionSizes(currentState);
	if (dimensions == nullptr) {
		errorMessage += "MarkovChain current state reference is invalid. ";
		return false;
	}
	if (dimensions->size() > 1u) {
		errorMessage += "MarkovChain current state must be scalar or one-dimensional. ";
		return false;
	}
	return true;
}

std::string formatDefinitionReference(const ModelDataDefinition* definition) {
	if (definition == nullptr) {
		return "";
	}
	return definition->getName();
}

std::string formatDefinitionPersistenceReference(const ModelDataDefinition* definition) {
	if (definition == nullptr) {
		return "";
	}
	return definition->getClassname() + ":" + definition->getName();
}

std::pair<std::string, std::string> parseDefinitionReference(const std::string& value) {
	const std::size_t separator = value.find(':');
	if (separator == std::string::npos) {
		const std::size_t suffixOpen = value.rfind(" (");
		if (suffixOpen != std::string::npos && !value.empty() && value.back() == ')') {
			return {value.substr(suffixOpen + 2, value.size() - suffixOpen - 3), value.substr(0, suffixOpen)};
		}
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
	bool supportsObjectCreation() const override { return true; }
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
					options->insert(definition->getName() + " (" + definition->getClassname() + ")");
				}
			}
		}
		return options;
	}
	List<std::string>* getCreatableReferenceTypes() override {
		List<std::string>* types = new List<std::string>();
		types->insert(Util::TypeOf<Attribute>());
		types->insert(Util::TypeOf<Variable>());
		return types;
	}
	std::string getCurrentReferenceType() const override {
		ModelDataDefinition* definition = getReferencedModelDataDefinition();
		return definition != nullptr ? definition->getClassname() : "";
	}
	bool createObjectInstance(const std::string& value = "") override {
		const std::string defaultType = getCurrentReferenceType().empty() ? Util::TypeOf<Attribute>() : getCurrentReferenceType();
		return createObjectInstanceOfType(defaultType, value);
	}
	bool createObjectInstanceOfType(const std::string& typeName, const std::string& value = "") override {
		_ensureWritable("create instance for");
		if (_setter == nullptr || _model == nullptr) {
			return false;
		}
		const std::string concreteType = typeName.empty() ? Util::TypeOf<Attribute>() : typeName;
		if (concreteType != Util::TypeOf<Attribute>() && concreteType != Util::TypeOf<Variable>()) {
			return false;
		}
		std::string name = value;
		if (name.empty()) {
			name = getValue();
		}
		if (name.empty()) {
			return false;
		}
		ModelDataDefinition* definition = _model->getDataManager()->getDataDefinition(concreteType, name);
		if (definition == nullptr) {
			definition = concreteType == Util::TypeOf<Attribute>()
			                 ? static_cast<ModelDataDefinition*>(new Attribute(_model, name))
			                 : static_cast<ModelDataDefinition*>(new Variable(_model, name));
		}
		_setter(definition);
		return hasObjectInstance();
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
			//static_cast<Attribute*>(definition)->setValue(value, index);
		}
		return;
	}
	throw std::invalid_argument("Unsupported MarkovChain data definition type.");
}

std::string currentStateIndex(const ModelDataDefinition* definition) {
	const std::list<unsigned int>* dimensions = getDefinitionDimensionSizes(definition);
	if (dimensions == nullptr || dimensions->empty()) {
		return "";
	}
	return "0";
}

unsigned int readCurrentStateIndex(ModelDataDefinition* definition, Entity* entity) {
	if (definition == nullptr) {
		return 0u;
	}
	const double value = readDefinitionValue(definition, entity, currentStateIndex(definition));
	return static_cast<unsigned int>(std::max(0.0, value));
}

void writeCurrentStateIndex(ModelDataDefinition* definition, Entity* entity, unsigned int value) {
	if (definition == nullptr) {
		return;
	}
	writeDefinitionValue(definition, entity, static_cast<double>(value), currentStateIndex(definition));
}

unsigned int sampleStateFromDistribution(const ModelDataDefinition* distribution,
                                         Sampler_if* sampler,
                                         unsigned int stateCount) {
	if (distribution == nullptr || sampler == nullptr || stateCount == 0u) {
		return 0u;
	}
	std::vector<double> probabilities;
	std::vector<double> values;
	probabilities.reserve(stateCount);
	values.reserve(stateCount);
	const std::list<unsigned int>* dimensions = getDefinitionDimensionSizes(distribution);
	for (unsigned int state = 0; state < stateCount; ++state) {
		const std::string index = (dimensions == nullptr || dimensions->empty()) ? "" : std::to_string(state);
		probabilities.push_back(readDefinitionValue(distribution, index));
		values.push_back(static_cast<double>(state));
	}
	return static_cast<unsigned int>(sampler->sampleDiscrete(probabilities.data(), values.data(), static_cast<int>(stateCount)));
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
	MarkovChainDataDefinitionReferenceControl* propInitialDistribution = new MarkovChainDataDefinitionReferenceControl(
			_parentModel,
			std::bind(&MarkovChain::getInitialDistribution, this),
			std::bind(&MarkovChain::setInitialDistribution, this, std::placeholders::_1),
			Util::TypeOf<MarkovChain>(), getName(), "InitialDistribution", "");

	_parentModel->getControls()->insert(propTransitionMatrix);
	_parentModel->getControls()->insert(propInitialDistribution);
	_addSimulationControl(propTransitionMatrix);
	_addSimulationControl(propInitialDistribution);

	MarkovChainDataDefinitionReferenceControl* propCurrentState = new MarkovChainDataDefinitionReferenceControl(
			_parentModel,
			std::bind(&MarkovChain::getCurrentState, this),
			std::bind(&MarkovChain::setCurrentState, this, std::placeholders::_1),
			Util::TypeOf<MarkovChain>(), getName(), "CurrentState", "");
	_parentModel->getControls()->insert(propCurrentState);
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

void MarkovChain::setInitialDistribution(ModelDataDefinition* initialDistribution) {
	if (!isMarkovChainSupportedDefinition(initialDistribution)) {
		throw std::invalid_argument("InitialDistribution must reference a Variable or an Attribute.");
	}
	_initialDistribution = initialDistribution;
}

ModelDataDefinition* MarkovChain::getInitialDistribution() const {
	return _initialDistribution;
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
	assert(entity != nullptr);
	assert(_transitionProbMatrix != nullptr);
	assert(_initialDistribution != nullptr);
	assert(_currentState != nullptr);
	const unsigned int currentState = readCurrentStateIndex(_currentState, entity);
	const unsigned int nextState = _drawNextState(entity, currentState);
	writeCurrentStateIndex(_currentState, entity, nextState);

		traceSimulation(this,
	                "MarkovChain sampled entity " + entity->getName() + " transition " + std::to_string(currentState) + " -> " +
	                    std::to_string(nextState) + " using " + formatDefinitionPersistenceReference(_currentState),
	                TraceManager::Level::L7_internal);
	_parentModel->sendEntityToComponent(entity, this->getConnectionManager()->getFrontConnection());
}

bool MarkovChain::_loadInstance(PersistenceRecord *fields) {
	const bool result = ModelComponent::_loadInstance(fields);
	if (result) {
		const std::string transitionReference = fields->loadField("transitionMatrixRef",
		                                                          fields->loadField("transitionMatrix", ""));
		const std::string initialDistributionReference = fields->loadField("initialDistributionRef",
		                                                                    fields->loadField("initialDistribution", ""));
		const std::string currentStateReference = fields->loadField("currentStateRef",
		                                                          fields->loadField("currentState", ""));
		setTransitionProbabilityMatrix(resolveDefinitionReference(_parentModel, transitionReference));
		setInitialDistribution(resolveDefinitionReference(_parentModel, initialDistributionReference));
		setCurrentState(resolveDefinitionReference(_parentModel, currentStateReference));
	}
	return result;
}

void MarkovChain::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	if (_transitionProbMatrix != nullptr) {
		fields->saveField("transitionMatrixRef", formatDefinitionPersistenceReference(_transitionProbMatrix), "", saveDefaultValues);
		fields->saveField("transitionMatrix", _transitionProbMatrix->getName(), "", saveDefaultValues);
	}
	if (_initialDistribution != nullptr) {
		fields->saveField("initialDistributionRef", formatDefinitionPersistenceReference(_initialDistribution), "", saveDefaultValues);
		fields->saveField("initialDistribution", _initialDistribution->getName(), "", saveDefaultValues);
	}
	if (_currentState != nullptr) {
		fields->saveField("currentStateRef", formatDefinitionPersistenceReference(_currentState), "", saveDefaultValues);
		fields->saveField("currentState", _currentState->getName(), "", saveDefaultValues);
	}
}

void MarkovChain::_initBetweenReplications() {
	if (_transitionProbMatrix == nullptr || _initialDistribution == nullptr || _currentState == nullptr) {
		return;
	}
	const unsigned int stateCount = _stateCount();
	if (stateCount == 0u) {
		return;
	}
	const unsigned int sampledState = sampleStateFromDistribution(_initialDistribution, _sampler, stateCount);
	writeCurrentStateIndex(_currentState, nullptr, sampledState);
}

bool MarkovChain::_check(std::string& errorMessage) {
	bool resultAll = true;
	if (_transitionProbMatrix == nullptr) {
		errorMessage += "MarkovChain requires a transition probability matrix. ";
		resultAll = false;
	}
	if (_initialDistribution == nullptr) {
		errorMessage += "MarkovChain requires an initial distribution reference. ";
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
	if (_transitionProbMatrix != nullptr && !validateTransitionMatrixDefinition(_transitionProbMatrix, errorMessage)) {
		resultAll = false;
	}
	if (_transitionProbMatrix != nullptr && _initialDistribution != nullptr &&
	    !validateInitialDistributionDefinition(_initialDistribution, _stateCount(), errorMessage)) {
		resultAll = false;
	}
	if (_currentState != nullptr && !validateCurrentStateDefinition(_currentState, errorMessage)) {
		resultAll = false;
	}
	return resultAll;
}


PluginInformation* MarkovChain::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<MarkovChain>(), &MarkovChain::LoadInstance, &MarkovChain::NewInstance);
	info->setCategory("AnalyticalModeling");
	return info;
}

// void MarkovChain::_createInternalStatisticReporters() { }

void MarkovChain::_createEditableDataDefinitions() {
	// _attachedDataClear(); //@TODO Check commented
	if (_transitionProbMatrix != nullptr) {
		_optionalEditableDataDefinitionInsert(_transitionProbMatrix->getName(), _transitionProbMatrix);
	}
	if (_initialDistribution != nullptr) {
		_optionalEditableDataDefinitionInsert(_initialDistribution->getName(), _initialDistribution);
	}
	if (_currentState != nullptr) {
		_optionalEditableDataDefinitionInsert(_currentState->getName(), _currentState);
	}
}

// void MarkovChain::_createAttachedAttributes() { }

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
	std::vector<double> probabilities;
	std::vector<double> values;
	probabilities.reserve(size);
	values.reserve(size);
	std::string message = "MarkovChain transition probability p[" + std::to_string(currentState) + "]= [";
	for (unsigned int nextState = 0; nextState < size; ++nextState) {
		const std::string index = std::to_string(currentState) + "," + std::to_string(nextState);
		const double probability = _readTransitionProbability(entity, currentState, nextState);
		probabilities.push_back(probability);
		values.push_back(static_cast<double>(nextState));
		message += std::to_string(probability)+",";
	}
	message += "]"; //@ToDo: (minor) remove extra ","
	traceSimulation(this, message, TraceManager::Level::L9_mostDetailed);
	return static_cast<unsigned int>(_sampler->sampleDiscrete(probabilities.data(), values.data(), static_cast<int>(size)));
}

unsigned int MarkovChain::_stateCount() const {
	const std::list<unsigned int>* dimensions = getDefinitionDimensionSizes(_transitionProbMatrix);
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
			    formatDefinitionPersistenceReference(_currentState),
			TraceManager::Level::L8_detailed);
	return value;
}

void MarkovChain::_writeStateValue(Entity* entity, double value) {
	writeDefinitionValue(_currentState, entity, value);
	traceSimulation(this,
	                "MarkovChain wrote current state " + std::to_string(value) + " to " +
	                    formatDefinitionPersistenceReference(_currentState),
	                TraceManager::Level::L8_detailed);
}

double MarkovChain::_readTransitionProbability(Entity* entity, unsigned int fromState, unsigned int toState) const {
	return readDefinitionValue(_transitionProbMatrix, entity,
	                           std::to_string(fromState) + "," + std::to_string(toState));
}
