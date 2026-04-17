#include "BioNetwork.h"

#include <algorithm>
#include <functional>
#include <iomanip>
#include <map>
#include <sstream>

#include "BioReaction.h"
#include "BioSpecies.h"
#include "../../kernel/simulator/Event.h"
#include "../../kernel/simulator/Model.h"
#include "../../kernel/simulator/ModelDataManager.h"
#include "../../tools/MassActionOdeSystem.h"
#include "../../tools/RungeKutta4OdeSolver.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &BioNetwork::GetPluginInformation;
}
#endif

namespace {

std::string jsonEscape(const std::string& value) {
	std::string escaped;
	for (char ch : value) {
		switch (ch) {
			case '\\':
				escaped += "\\\\";
				break;
			case '"':
				escaped += "\\\"";
				break;
			case '\n':
				escaped += "\\n";
				break;
			case '\r':
				escaped += "\\r";
				break;
			case '\t':
				escaped += "\\t";
				break;
			default:
				escaped += ch;
				break;
		}
	}
	return escaped;
}

std::string formatDouble(double value) {
	std::ostringstream out;
	out << std::setprecision(15) << value;
	return out.str();
}

} // namespace

ModelDataDefinition* BioNetwork::NewInstance(Model* model, std::string name) {
	return new BioNetwork(model, name);
}

BioNetwork::BioNetwork(Model* model, std::string name) : ModelDataDefinition(model, Util::TypeOf<BioNetwork>(), name) {
	auto* propStartTime = new SimulationControlDouble(
			std::bind(&BioNetwork::getStartTime, this), std::bind(&BioNetwork::setStartTime, this, std::placeholders::_1),
			Util::TypeOf<BioNetwork>(), getName(), "StartTime", "");
	auto* propStopTime = new SimulationControlDouble(
			std::bind(&BioNetwork::getStopTime, this), std::bind(&BioNetwork::setStopTime, this, std::placeholders::_1),
			Util::TypeOf<BioNetwork>(), getName(), "StopTime", "");
	auto* propStepSize = new SimulationControlDouble(
			std::bind(&BioNetwork::getStepSize, this), std::bind(&BioNetwork::setStepSize, this, std::placeholders::_1),
			Util::TypeOf<BioNetwork>(), getName(), "StepSize", "");
	auto* propCurrentTime = new SimulationControlDouble(
			std::bind(&BioNetwork::getCurrentTime, this), std::bind(&BioNetwork::setCurrentTime, this, std::placeholders::_1),
			Util::TypeOf<BioNetwork>(), getName(), "CurrentTime", "");
	auto* propAutoSchedule = new SimulationControlGeneric<bool>(
			std::bind(&BioNetwork::getAutoSchedule, this), std::bind(&BioNetwork::setAutoSchedule, this, std::placeholders::_1),
			Util::TypeOf<BioNetwork>(), getName(), "AutoSchedule", "");
	auto* propLastStatus = new SimulationControlGeneric<std::string>(
			std::bind(&BioNetwork::getLastStatus, this), std::bind(&BioNetwork::setLastStatus, this, std::placeholders::_1),
			Util::TypeOf<BioNetwork>(), getName(), "LastStatus", "");
	auto* propLastErrorMessage = new SimulationControlGeneric<std::string>(
			std::bind(&BioNetwork::getLastErrorMessage, this), std::bind(&BioNetwork::setLastErrorMessage, this, std::placeholders::_1),
			Util::TypeOf<BioNetwork>(), getName(), "LastErrorMessage", "");
	auto* propLastResponsePayload = new SimulationControlGeneric<std::string>(
			std::bind(&BioNetwork::getLastResponsePayload, this), std::bind(&BioNetwork::setLastResponsePayload, this, std::placeholders::_1),
			Util::TypeOf<BioNetwork>(), getName(), "LastResponsePayload", "");

	_parentModel->getControls()->insert(propStartTime);
	_parentModel->getControls()->insert(propStopTime);
	_parentModel->getControls()->insert(propStepSize);
	_parentModel->getControls()->insert(propCurrentTime);
	_parentModel->getControls()->insert(propAutoSchedule);
	_parentModel->getControls()->insert(propLastStatus);
	_parentModel->getControls()->insert(propLastErrorMessage);
	_parentModel->getControls()->insert(propLastResponsePayload);

	_addProperty(propStartTime);
	_addProperty(propStopTime);
	_addProperty(propStepSize);
	_addProperty(propCurrentTime);
	_addProperty(propAutoSchedule);
	_addProperty(propLastStatus);
	_addProperty(propLastErrorMessage);
	_addProperty(propLastResponsePayload);
}

PluginInformation* BioNetwork::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<BioNetwork>(), &BioNetwork::LoadInstance, &BioNetwork::NewInstance);
	info->setCategory("Biochemical simulation");
	info->setDescriptionHelp("Native biochemical network runner. It discovers BioSpecies and BioReaction data definitions in the model and advances them with mass-action kinetics using a fixed-step RK4 solver.");
	return info;
}

ModelDataDefinition* BioNetwork::LoadInstance(Model* model, PersistenceRecord *fields) {
	BioNetwork* newElement = new BioNetwork(model);
	try {
		newElement->_loadInstance(fields);
	} catch (const std::exception& e) {
	}
	return newElement;
}

std::string BioNetwork::show() {
	return ModelDataDefinition::show() +
			",startTime=" + Util::StrTruncIfInt(std::to_string(_startTime)) +
			",stopTime=" + Util::StrTruncIfInt(std::to_string(_stopTime)) +
			",stepSize=" + Util::StrTruncIfInt(std::to_string(_stepSize)) +
			",currentTime=" + Util::StrTruncIfInt(std::to_string(_currentTime)) +
			",autoSchedule=" + std::to_string(_autoSchedule ? 1 : 0) +
			",lastStatus=\"" + _lastStatus + "\"" +
			",lastResponsePayloadSize=" + std::to_string(_lastResponsePayload.size());
}

bool BioNetwork::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelDataDefinition::_loadInstance(fields);
	if (res) {
		_startTime = fields->loadField("startTime", DEFAULT.startTime);
		_stopTime = fields->loadField("stopTime", DEFAULT.stopTime);
		_stepSize = fields->loadField("stepSize", DEFAULT.stepSize);
		_currentTime = fields->loadField("currentTime", _startTime);
		_autoSchedule = fields->loadField("autoSchedule", DEFAULT.autoSchedule ? 1u : 0u) != 0u;
		_lastStatus = fields->loadField("lastStatus", DEFAULT.lastStatus);
		_lastErrorMessage = fields->loadField("lastErrorMessage", DEFAULT.lastErrorMessage);
		_lastResponsePayload = fields->loadField("lastResponsePayload", DEFAULT.lastResponsePayload);
	}
	return res;
}

void BioNetwork::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelDataDefinition::_saveInstance(fields, saveDefaultValues);
	fields->saveField("startTime", _startTime, DEFAULT.startTime, saveDefaultValues);
	fields->saveField("stopTime", _stopTime, DEFAULT.stopTime, saveDefaultValues);
	fields->saveField("stepSize", _stepSize, DEFAULT.stepSize, saveDefaultValues);
	fields->saveField("currentTime", _currentTime, DEFAULT.currentTime, saveDefaultValues);
	fields->saveField("autoSchedule", _autoSchedule ? 1u : 0u, DEFAULT.autoSchedule ? 1u : 0u, saveDefaultValues);
	fields->saveField("lastStatus", _lastStatus, DEFAULT.lastStatus, saveDefaultValues);
	fields->saveField("lastErrorMessage", _lastErrorMessage, DEFAULT.lastErrorMessage, saveDefaultValues);
	fields->saveField("lastResponsePayload", _lastResponsePayload, DEFAULT.lastResponsePayload, saveDefaultValues);
}

bool BioNetwork::_check(std::string& errorMessage) {
	bool resultAll = true;
	if (getName().empty()) {
		errorMessage += "BioNetwork must define a non-empty name. ";
		resultAll = false;
	}
	if (_stepSize <= 0.0) {
		errorMessage += "BioNetwork \"" + getName() + "\" must define stepSize > 0. ";
		resultAll = false;
	}
	if (_stopTime < _startTime) {
		errorMessage += "BioNetwork \"" + getName() + "\" must define stopTime >= startTime. ";
		resultAll = false;
	}

	std::vector<BioSpecies*> species;
	std::vector<BioReaction*> reactions;
	resultAll = collectSpecies(species, errorMessage) && resultAll;
	resultAll = collectReactions(reactions, errorMessage) && resultAll;
	return resultAll;
}

void BioNetwork::_initBetweenReplications() {
	_currentTime = _startTime;
	_lastStatus = "Idle";
	_lastErrorMessage.clear();
	_lastResponsePayload.clear();
	if (_autoSchedule) {
		scheduleNextInternalEvent();
	}
}

bool BioNetwork::simulate(std::string& errorMessage) {
	return simulate(_startTime, _stopTime, _stepSize, errorMessage);
}

bool BioNetwork::simulate(double startTime, double stopTime, double stepSize, std::string& errorMessage) {
	_startTime = startTime;
	_stopTime = stopTime;
	_stepSize = stepSize;
	_currentTime = _startTime;
	_lastStatus = "Running";
	_lastErrorMessage.clear();
	_lastResponsePayload.clear();

	if (!_check(errorMessage)) {
		_lastStatus = "Failed";
		_lastErrorMessage = errorMessage;
		return false;
	}

	while (_currentTime < _stopTime) {
		if (!advanceOneStep(errorMessage)) {
			return false;
		}
	}
	_lastStatus = "Completed";
	return true;
}

bool BioNetwork::advanceOneStep(std::string& errorMessage) {
	errorMessage.clear();
	if (_stepSize <= 0.0) {
		_lastStatus = "Failed";
		_lastErrorMessage = "BioNetwork \"" + getName() + "\" must define stepSize > 0.";
		errorMessage = _lastErrorMessage;
		return false;
	}
	if (_currentTime >= _stopTime) {
		_lastStatus = "Completed";
		return true;
	}

	std::vector<BioSpecies*> species;
	std::vector<BioReaction*> reactions;
	if (!collectSpecies(species, errorMessage) || !collectReactions(reactions, errorMessage)) {
		_lastStatus = "Failed";
		_lastErrorMessage = errorMessage;
		return false;
	}

	MassActionOdeSystem system;
	if (!buildSystem(species, reactions, &system, errorMessage)) {
		_lastStatus = "Failed";
		_lastErrorMessage = errorMessage;
		return false;
	}

	const double dt = std::min(_stepSize, _stopTime - _currentTime);
	std::vector<double> y0(species.size(), 0.0);
	std::vector<double> y1(species.size(), 0.0);
	for (unsigned int i = 0; i < species.size(); ++i) {
		y0[i] = species[i]->getAmount();
	}

	RungeKutta4OdeSolver solver;
	if (!solver.advance(system, _currentTime, dt, y0.data(), y1.data())) {
		_lastStatus = "Failed";
		_lastErrorMessage = "BioNetwork \"" + getName() + "\" failed to advance the ODE system.";
		errorMessage = _lastErrorMessage;
		return false;
	}

	for (unsigned int i = 0; i < species.size(); ++i) {
		if (!species[i]->isBoundaryCondition() && !species[i]->isConstant()) {
			species[i]->setAmount(std::max(0.0, y1[i]));
		}
	}
	_currentTime += dt;
	_lastStatus = _currentTime >= _stopTime ? "Completed" : "Running";
	updatePayload(species);
	return true;
}

bool BioNetwork::collectSpecies(std::vector<BioSpecies*>& species, std::string& errorMessage) const {
	species.clear();
	List<ModelDataDefinition*>* list = _parentModel->getDataManager()->getDataDefinitionList(Util::TypeOf<BioSpecies>());
	for (ModelDataDefinition* definition : *list->list()) {
		auto* bioSpecies = dynamic_cast<BioSpecies*>(definition);
		if (bioSpecies != nullptr) {
			species.push_back(bioSpecies);
		}
	}
	if (species.empty()) {
		errorMessage += "BioNetwork \"" + getName() + "\" requires at least one BioSpecies. ";
		return false;
	}
	return true;
}

bool BioNetwork::collectReactions(std::vector<BioReaction*>& reactions, std::string& errorMessage) const {
	reactions.clear();
	List<ModelDataDefinition*>* list = _parentModel->getDataManager()->getDataDefinitionList(Util::TypeOf<BioReaction>());
	for (ModelDataDefinition* definition : *list->list()) {
		auto* reaction = dynamic_cast<BioReaction*>(definition);
		if (reaction != nullptr) {
			reactions.push_back(reaction);
		}
	}
	if (reactions.empty()) {
		errorMessage += "BioNetwork \"" + getName() + "\" requires at least one BioReaction. ";
		return false;
	}
	return true;
}

bool BioNetwork::buildSystem(const std::vector<BioSpecies*>& species, const std::vector<BioReaction*>& reactions, MassActionOdeSystem* system, std::string& errorMessage) const {
	if (system == nullptr) {
		errorMessage += "BioNetwork \"" + getName() + "\" received a null ODE system. ";
		return false;
	}

	std::vector<MassActionOdeSystem::Species> odeSpecies;
	std::map<std::string, unsigned int> indexes;
	for (unsigned int i = 0; i < species.size(); ++i) {
		odeSpecies.push_back({
				species[i]->getName(),
				species[i]->getAmount(),
				species[i]->isBoundaryCondition(),
				species[i]->isConstant()
		});
		indexes[species[i]->getName()] = i;
	}

	std::vector<MassActionOdeSystem::Reaction> odeReactions;
	for (BioReaction* reaction : reactions) {
		if (reaction->isReversible()) {
			errorMessage += "BioNetwork \"" + getName() + "\" cannot run reversible BioReaction \"" + reaction->getName() + "\" yet. ";
			return false;
		}
		const double rateConstant = reaction->resolveRateConstant();
		if (rateConstant < 0.0) {
			errorMessage += "BioReaction \"" + reaction->getName() + "\" must resolve to a non-negative rate constant. ";
			return false;
		}

		MassActionOdeSystem::Reaction odeReaction;
		odeReaction.name = reaction->getName();
		odeReaction.rateConstant = rateConstant;

		for (const BioReaction::StoichiometricTerm& term : reaction->getReactants()) {
			auto it = indexes.find(term.speciesName);
			if (it == indexes.end()) {
				errorMessage += "BioReaction \"" + reaction->getName() + "\" references missing reactant BioSpecies \"" + term.speciesName + "\". ";
				return false;
			}
			odeReaction.reactants.push_back({it->second, term.stoichiometry});
		}
		for (const BioReaction::StoichiometricTerm& term : reaction->getProducts()) {
			auto it = indexes.find(term.speciesName);
			if (it == indexes.end()) {
				errorMessage += "BioReaction \"" + reaction->getName() + "\" references missing product BioSpecies \"" + term.speciesName + "\". ";
				return false;
			}
			odeReaction.products.push_back({it->second, term.stoichiometry});
		}
		odeReactions.push_back(odeReaction);
	}

	*system = MassActionOdeSystem(odeSpecies, odeReactions);
	return true;
}

void BioNetwork::updatePayload(const std::vector<BioSpecies*>& species) {
	std::string payload = "{\"status\":\"" + jsonEscape(_lastStatus) + "\"" +
			",\"time\":" + formatDouble(_currentTime) +
			",\"species\":{";
	for (const BioSpecies* bioSpecies : species) {
		payload += "\"" + jsonEscape(bioSpecies->getName()) + "\":" + formatDouble(bioSpecies->getAmount()) + ",";
	}
	if (!species.empty()) {
		payload.pop_back();
	}
	payload += "}}";
	_lastResponsePayload = payload;
}

void BioNetwork::handleInternalEvent(void*) {
	std::string errorMessage;
	if (advanceOneStep(errorMessage) && _autoSchedule && _currentTime < _stopTime) {
		scheduleNextInternalEvent();
	}
}

void BioNetwork::scheduleNextInternalEvent() {
	if (_stepSize <= 0.0 || _currentTime >= _stopTime) {
		return;
	}
	const double eventTime = std::min(_currentTime + _stepSize, _stopTime);
	auto* event = new InternalEvent(eventTime, "BioNetworkStep");
	event->setEventHandler(this, &BioNetwork::handleInternalEvent, nullptr);
	_parentModel->getFutureEvents()->insert(event);
}

void BioNetwork::setStartTime(double startTime) {
	_startTime = startTime;
}

double BioNetwork::getStartTime() const {
	return _startTime;
}

void BioNetwork::setStopTime(double stopTime) {
	_stopTime = stopTime;
}

double BioNetwork::getStopTime() const {
	return _stopTime;
}

void BioNetwork::setStepSize(double stepSize) {
	_stepSize = stepSize;
}

double BioNetwork::getStepSize() const {
	return _stepSize;
}

void BioNetwork::setCurrentTime(double currentTime) {
	_currentTime = currentTime;
}

double BioNetwork::getCurrentTime() const {
	return _currentTime;
}

void BioNetwork::setAutoSchedule(bool autoSchedule) {
	_autoSchedule = autoSchedule;
}

bool BioNetwork::getAutoSchedule() const {
	return _autoSchedule;
}

void BioNetwork::setLastStatus(std::string lastStatus) {
	_lastStatus = lastStatus;
}

std::string BioNetwork::getLastStatus() const {
	return _lastStatus;
}

void BioNetwork::setLastErrorMessage(std::string lastErrorMessage) {
	_lastErrorMessage = lastErrorMessage;
}

std::string BioNetwork::getLastErrorMessage() const {
	return _lastErrorMessage;
}

void BioNetwork::setLastResponsePayload(std::string lastResponsePayload) {
	_lastResponsePayload = lastResponsePayload;
}

std::string BioNetwork::getLastResponsePayload() const {
	return _lastResponsePayload;
}
