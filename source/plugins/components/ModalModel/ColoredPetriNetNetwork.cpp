#include "plugins/data/ModalModel/ColoredPetriNetNetwork.h"

#include "kernel/simulator/PluginManager.h"
#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/model/Model.h"

#include <algorithm>
#include <memory>
#include <unordered_map>

#ifdef PLUGINCONNECT_DYNAMIC
extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &ColoredPetriNetNetwork::GetPluginInformation;
}
#endif

ColoredPetriNetNetwork::ColoredPetriNetNetwork(Model* model, std::string name)
	: DefaultNetwork(model, name, Util::TypeOf<ColoredPetriNetNetwork>()) {
	addInputPort("fire");
	addOutputPort("fired");
}

ColoredPetriNetNetwork::~ColoredPetriNetNetwork() {
	delete _places;
	delete _transitions;
	delete _arcs;
}

ModelDataDefinition* ColoredPetriNetNetwork::NewInstance(Model* model, std::string name) {
	return new ColoredPetriNetNetwork(model, name);
}

PluginInformation* ColoredPetriNetNetwork::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<ColoredPetriNetNetwork>(), &ColoredPetriNetNetwork::LoadInstance, &ColoredPetriNetNetwork::NewInstance);
	info->setCategory("ModalModel");
	info->setDescriptionHelp("Fixed-inscription coloured Petri net network with bipartite topology and deterministic single firing.");
	return info;
}

ModelDataDefinition* ColoredPetriNetNetwork::LoadInstance(Model* model, PersistenceRecord* fields) {
	ColoredPetriNetNetwork* network = new ColoredPetriNetNetwork(model);
	try {
		network->_loadInstance(fields);
	} catch (const std::exception& e) {
		network->traceError("Failed to load ColoredPetriNetNetwork instance: " + std::string(e.what()));
	}
	return network;
}

void ColoredPetriNetNetwork::addPlace(PetriPlace* place) {
	if (place != nullptr && !_hasPlace(place)) {
		_places->insert(place);
		if (_initialMarking.find(place) == _initialMarking.end()) {
			_initialMarking[place] = *place->getAllTokens();
		}
	}
}

void ColoredPetriNetNetwork::removePlace(PetriPlace* place) {
	if (place == nullptr) {
		return;
	}
	_removeIncidentArcs(place);
	_initialMarking.erase(place);
	_places->remove(place);
}

List<PetriPlace*>* ColoredPetriNetNetwork::getPlaces() const {
	return _places;
}

void ColoredPetriNetNetwork::addTransition(CPNTransition* transition) {
	if (transition != nullptr && !_hasTransition(transition)) {
		_transitions->insert(transition);
	}
}

void ColoredPetriNetNetwork::removeTransition(CPNTransition* transition) {
	if (transition == nullptr) {
		return;
	}
	_removeIncidentArcs(transition);
	_transitions->remove(transition);
}

List<CPNTransition*>* ColoredPetriNetNetwork::getTransitions() const {
	return _transitions;
}

bool ColoredPetriNetNetwork::addArc(CPNArc* arc) {
	if (arc == nullptr || _hasArc(arc) || !_hasPlace(arc->getPlace()) || !_hasTransition(arc->getTransition())) {
		return false;
	}
	_arcs->insert(arc);
	return true;
}

void ColoredPetriNetNetwork::removeArc(CPNArc* arc) {
	_arcs->remove(arc);
}

List<CPNArc*>* ColoredPetriNetNetwork::getArcs() const {
	return _arcs;
}

std::vector<CPNArc*> ColoredPetriNetNetwork::getInputArcs(CPNTransition* transition) const {
	std::vector<CPNArc*> result;
	for (CPNArc* arc : *_arcs->list()) {
		if (arc != nullptr && arc->getTransition() == transition && arc->isInputArc()) {
			result.push_back(arc);
		}
	}
	return result;
}

std::vector<CPNArc*> ColoredPetriNetNetwork::getOutputArcs(CPNTransition* transition) const {
	std::vector<CPNArc*> result;
	for (CPNArc* arc : *_arcs->list()) {
		if (arc != nullptr && arc->getTransition() == transition && arc->isOutputArc()) {
			result.push_back(arc);
		}
	}
	return result;
}

bool ColoredPetriNetNetwork::isEnabled(CPNTransition* transition) const {
	if (!_hasTransition(transition) || !_guardAllows(transition)) {
		return false;
	}
	for (CPNArc* arc : getInputArcs(transition)) {
		PetriPlace* place = arc != nullptr ? arc->getPlace() : nullptr;
		if (place == nullptr) {
			return false;
		}
		for (const auto& inscription : arc->getInscriptions()) {
			if (place->getTokens(inscription.first) < inscription.second) {
				return false;
			}
		}
	}
	return !getInputArcs(transition).empty() || !getOutputArcs(transition).empty();
}

CPNTransition* ColoredPetriNetNetwork::firstEnabledTransition() const {
	std::vector<CPNTransition*> enabled;
	for (CPNTransition* transition : *_transitions->list()) {
		if (isEnabled(transition)) {
			enabled.push_back(transition);
		}
	}
	if (enabled.empty()) {
		return nullptr;
	}
	std::stable_sort(enabled.begin(), enabled.end(), [](CPNTransition* a, CPNTransition* b) {
		return a->getPriority() < b->getPriority();
	});
	return enabled.front();
}

bool ColoredPetriNetNetwork::fire(CPNTransition* transition) {
	if (!isEnabled(transition)) {
		return false;
	}
	for (CPNArc* arc : getInputArcs(transition)) {
		PetriPlace* place = arc->getPlace();
		for (const auto& inscription : arc->getInscriptions()) {
			if (!place->removeTokens(inscription.second, inscription.first)) {
				return false;
			}
		}
	}
	for (CPNArc* arc : getOutputArcs(transition)) {
		PetriPlace* place = arc->getPlace();
		for (const auto& inscription : arc->getInscriptions()) {
			place->addTokens(inscription.second, inscription.first);
		}
	}
	return true;
}

void ColoredPetriNetNetwork::setInitialTokens(PetriPlace* place, std::string color, unsigned int quantity) {
	if (place == nullptr) {
		return;
	}
	addPlace(place);
	_initialMarking[place][color] = quantity;
	place->setTokens(quantity, color);
}

unsigned int ColoredPetriNetNetwork::getInitialTokens(PetriPlace* place, std::string color) const {
	auto placeIt = _initialMarking.find(place);
	if (placeIt == _initialMarking.end()) {
		return 0u;
	}
	auto colorIt = placeIt->second.find(color);
	return colorIt != placeIt->second.end() ? colorIt->second : 0u;
}

void ColoredPetriNetNetwork::captureCurrentMarkingAsInitial() {
	_initialMarking.clear();
	for (PetriPlace* place : *_places->list()) {
		if (place != nullptr) {
			_initialMarking[place] = *place->getAllTokens();
		}
	}
}

void ColoredPetriNetNetwork::restoreInitialMarking() {
	for (PetriPlace* place : *_places->list()) {
		if (place == nullptr) {
			continue;
		}
		place->clearTokens();
		auto placeIt = _initialMarking.find(place);
		if (placeIt != _initialMarking.end()) {
			for (const auto& token : placeIt->second) {
				place->setTokens(token.second, token.first);
			}
		}
	}
}

ColoredPetriNetNetwork::FiringMode ColoredPetriNetNetwork::getFiringMode() const {
	return _firingMode;
}

void ColoredPetriNetNetwork::setFiringMode(FiringMode firingMode) {
	_firingMode = firingMode;
}

std::string ColoredPetriNetNetwork::show() {
	return DefaultNetwork::show() +
	       ", places=" + std::to_string(_places->size()) +
	       ", transitions=" + std::to_string(_transitions->size()) +
	       ", arcs=" + std::to_string(_arcs->size()) +
	       ", firingMode=" + std::to_string(static_cast<unsigned int>(_firingMode));
}

bool ColoredPetriNetNetwork::_loadInstance(PersistenceRecord* fields) {
	bool res = DefaultNetwork::_loadInstance(fields);
	if (res) {
		_places->clear();
		_transitions->clear();
		_arcs->clear();
		_initialMarking.clear();
		_firingMode = static_cast<FiringMode>(fields->loadField("firingMode", static_cast<unsigned int>(FiringMode::SingleDeterministic)));

		PluginManager* plugins = _parentModel->getParentSimulator()->getPluginManager();
		std::unordered_map<std::string, PetriPlace*> placesByName;
		std::unordered_map<std::string, CPNTransition*> transitionsByName;

		unsigned int placesSize = fields->loadField("cpnPlacesSize", 0u);
		for (unsigned int i = 0; i < placesSize; i++) {
			const std::string prefix = "cpnPlace" + Util::StrIndex(i) + ".";
			auto placeFields = std::unique_ptr<PersistenceRecord>(fields->newInstance());
			for (auto it = fields->begin(); it != fields->end(); ++it) {
				if (it->first.rfind(prefix, 0) == 0) {
					PersistenceRecord::Entry entry = it->second;
					entry.first = it->first.substr(prefix.size());
					placeFields->insert(entry);
				}
			}
			if (placeFields->size() == 0) {
				continue;
			}
			ModelDataDefinition* loaded = nullptr;
			Plugin* placePlugin = plugins != nullptr ? plugins->find(placeFields->loadField("typename", Util::TypeOf<PetriPlace>())) : nullptr;
			if (placePlugin != nullptr) {
				loaded = placePlugin->loadNew(_parentModel, placeFields.get());
			} else {
				loaded = PetriPlace::LoadInstance(_parentModel, placeFields.get());
			}
			PetriPlace* place = dynamic_cast<PetriPlace*>(loaded);
			if (place == nullptr) {
				traceError("Loaded CPN place is not a PetriPlace while loading \"" + getName() + "\"");
				continue;
			}
			place->setModelLevel(_id);
			addPlace(place);
			placesByName[place->getName()] = place;
		}

		unsigned int transitionsSize = fields->loadField("cpnTransitionsSize", 0u);
		for (unsigned int i = 0; i < transitionsSize; i++) {
			const std::string prefix = "cpnTransition" + Util::StrIndex(i) + ".";
			auto transitionFields = std::unique_ptr<PersistenceRecord>(fields->newInstance());
			for (auto it = fields->begin(); it != fields->end(); ++it) {
				if (it->first.rfind(prefix, 0) == 0) {
					PersistenceRecord::Entry entry = it->second;
					entry.first = it->first.substr(prefix.size());
					transitionFields->insert(entry);
				}
			}
			if (transitionFields->size() == 0) {
				continue;
			}
			ModelDataDefinition* loaded = nullptr;
			Plugin* transitionPlugin = plugins != nullptr ? plugins->find(transitionFields->loadField("typename", Util::TypeOf<CPNTransition>())) : nullptr;
			if (transitionPlugin != nullptr) {
				loaded = transitionPlugin->loadNew(_parentModel, transitionFields.get());
			} else {
				loaded = CPNTransition::LoadInstance(_parentModel, transitionFields.get());
			}
			CPNTransition* transition = dynamic_cast<CPNTransition*>(loaded);
			if (transition == nullptr) {
				traceError("Loaded CPN transition is not a CPNTransition while loading \"" + getName() + "\"");
				continue;
			}
			transition->setModelLevel(_id);
			addTransition(transition);
			transitionsByName[transition->getName()] = transition;
		}

		unsigned int arcsSize = fields->loadField("cpnArcsSize", 0u);
		for (unsigned int i = 0; i < arcsSize; i++) {
			const std::string prefix = "cpnArc" + Util::StrIndex(i) + ".";
			auto arcFields = std::unique_ptr<PersistenceRecord>(fields->newInstance());
			for (auto it = fields->begin(); it != fields->end(); ++it) {
				if (it->first.rfind(prefix, 0) == 0) {
					PersistenceRecord::Entry entry = it->second;
					entry.first = it->first.substr(prefix.size());
					arcFields->insert(entry);
				}
			}
			if (arcFields->size() == 0) {
				continue;
			}
			CPNArc* arc = dynamic_cast<CPNArc*>(CPNArc::LoadInstance(_parentModel, arcFields.get()));
			if (arc == nullptr) {
				continue;
			}
			auto placeIt = placesByName.find(arc->getPlaceName());
			auto transitionIt = transitionsByName.find(arc->getTransitionName());
			if (placeIt == placesByName.end() || transitionIt == transitionsByName.end()) {
				traceError("Skipping CPN arc with unknown place/transition while loading \"" + getName() + "\"");
				continue;
			}
			arc->setPlace(placeIt->second);
			arc->setTransition(transitionIt->second);
			arc->setModelLevel(_id);
			addArc(arc);
		}

		unsigned int initialMarkingSize = fields->loadField("initialMarkingSize", 0u);
		for (unsigned int i = 0; i < initialMarkingSize; i++) {
			const std::string suffix = Util::StrIndex(i);
			auto placeIt = placesByName.find(fields->loadField("initialMarkingPlace" + suffix, std::string("")));
			const std::string color = fields->loadField("initialMarkingColor" + suffix, std::string("default"));
			const unsigned int quantity = fields->loadField("initialMarkingQuantity" + suffix, 0u);
			if (placeIt != placesByName.end()) {
				_initialMarking[placeIt->second][color] = quantity;
			}
		}
	}
	return res;
}

void ColoredPetriNetNetwork::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
	DefaultNetwork::_saveInstance(fields, saveDefaultValues);
	fields->saveField("firingMode", static_cast<unsigned int>(_firingMode), static_cast<unsigned int>(FiringMode::SingleDeterministic), saveDefaultValues);

	fields->saveField("cpnPlacesSize", _places->size(), 0u, saveDefaultValues);
	unsigned int i = 0;
	for (PetriPlace* place : *_places->list()) {
		const std::string prefix = "cpnPlace" + Util::StrIndex(i) + ".";
		auto placeFields = std::unique_ptr<PersistenceRecord>(fields->newInstance());
		ModelDataDefinition::SaveInstance(placeFields.get(), place);
		for (auto it = placeFields->begin(); it != placeFields->end(); ++it) {
			PersistenceRecord::Entry entry = it->second;
			entry.first = prefix + it->first;
			fields->insert(entry);
		}
		i++;
	}

	fields->saveField("cpnTransitionsSize", _transitions->size(), 0u, saveDefaultValues);
	i = 0;
	for (CPNTransition* transition : *_transitions->list()) {
		const std::string prefix = "cpnTransition" + Util::StrIndex(i) + ".";
		auto transitionFields = std::unique_ptr<PersistenceRecord>(fields->newInstance());
		ModelDataDefinition::SaveInstance(transitionFields.get(), transition);
		for (auto it = transitionFields->begin(); it != transitionFields->end(); ++it) {
			PersistenceRecord::Entry entry = it->second;
			entry.first = prefix + it->first;
			fields->insert(entry);
		}
		i++;
	}

	fields->saveField("cpnArcsSize", _arcs->size(), 0u, saveDefaultValues);
	i = 0;
	for (CPNArc* arc : *_arcs->list()) {
		const std::string prefix = "cpnArc" + Util::StrIndex(i) + ".";
		auto arcFields = std::unique_ptr<PersistenceRecord>(fields->newInstance());
		ModelDataDefinition::SaveInstance(arcFields.get(), arc);
		for (auto it = arcFields->begin(); it != arcFields->end(); ++it) {
			PersistenceRecord::Entry entry = it->second;
			entry.first = prefix + it->first;
			fields->insert(entry);
		}
		i++;
	}

	unsigned int markingSize = 0u;
	for (const auto& placeMarking : _initialMarking) {
		markingSize += placeMarking.second.size();
	}
	fields->saveField("initialMarkingSize", markingSize, 0u, saveDefaultValues);
	i = 0;
	for (const auto& placeMarking : _initialMarking) {
		for (const auto& token : placeMarking.second) {
			const std::string suffix = Util::StrIndex(i);
			fields->saveField("initialMarkingPlace" + suffix, placeMarking.first != nullptr ? placeMarking.first->getName() : std::string(""), std::string(""), saveDefaultValues);
			fields->saveField("initialMarkingColor" + suffix, token.first, std::string("default"), saveDefaultValues);
			fields->saveField("initialMarkingQuantity" + suffix, token.second, 0u, saveDefaultValues);
			i++;
		}
	}
}

bool ColoredPetriNetNetwork::_check(std::string& errorMessage) {
	bool resultAll = DefaultNetwork::_check(errorMessage);
	if (_firingMode != FiringMode::SingleDeterministic) {
		errorMessage += "ColoredPetriNetNetwork \"" + getName() + "\" supports only SingleDeterministic firing in this subset. ";
		resultAll = false;
	}
	if (_places->size() == 0) {
		errorMessage += "ColoredPetriNetNetwork \"" + getName() + "\" requires at least one place. ";
		resultAll = false;
	}
	if (_transitions->size() == 0) {
		errorMessage += "ColoredPetriNetNetwork \"" + getName() + "\" requires at least one transition. ";
		resultAll = false;
	}
	for (CPNTransition* transition : *_transitions->list()) {
		if (transition == nullptr) {
			errorMessage += "ColoredPetriNetNetwork \"" + getName() + "\" has a null transition reference. ";
			resultAll = false;
			continue;
		}
		if (transition->getGuardExpression() != "" && !_parentModel->checkExpression(transition->getGuardExpression(), "CPN transition guard[" + transition->getName() + "]", errorMessage)) {
			resultAll = false;
		}
	}
	for (CPNArc* arc : *_arcs->list()) {
		if (arc == nullptr) {
			errorMessage += "ColoredPetriNetNetwork \"" + getName() + "\" has a null arc reference. ";
			resultAll = false;
			continue;
		}
		if (!_hasPlace(arc->getPlace()) || !_hasTransition(arc->getTransition())) {
			errorMessage += "ColoredPetriNetNetwork \"" + getName() + "\" has arc \"" + arc->getName() + "\" outside the bipartite topology. ";
			resultAll = false;
		}
		if (arc->getInscriptions().empty()) {
			errorMessage += "ColoredPetriNetNetwork \"" + getName() + "\" has arc \"" + arc->getName() + "\" without fixed inscription. ";
			resultAll = false;
		}
		for (const auto& inscription : arc->getInscriptions()) {
			if (inscription.first.empty() || inscription.second == 0u) {
				errorMessage += "ColoredPetriNetNetwork \"" + getName() + "\" has invalid inscription on arc \"" + arc->getName() + "\". ";
				resultAll = false;
			}
		}
	}
	return resultAll;
}

void ColoredPetriNetNetwork::_initBetweenReplications() {
	DefaultNetwork::_initBetweenReplications();
	restoreInitialMarking();
}

NetworkActivationResult ColoredPetriNetNetwork::_activate(const NetworkActivationFrame& frame) {
	(void)frame;
	NetworkActivationResult result(getNumOutputPorts());
	CPNTransition* transition = firstEnabledTransition();
	if (transition != nullptr && fire(transition) && getNumOutputPorts() > 0) {
		result.setPresent(0, 1.0);
	}
	return result;
}

bool ColoredPetriNetNetwork::_hasPlace(PetriPlace* place) const {
	return place != nullptr && _places->find(place) != _places->list()->end();
}

bool ColoredPetriNetNetwork::_hasTransition(CPNTransition* transition) const {
	return transition != nullptr && _transitions->find(transition) != _transitions->list()->end();
}

bool ColoredPetriNetNetwork::_hasArc(CPNArc* arc) const {
	return arc != nullptr && _arcs->find(arc) != _arcs->list()->end();
}

bool ColoredPetriNetNetwork::_guardAllows(CPNTransition* transition) const {
	if (transition == nullptr || transition->getGuardExpression() == "") {
		return true;
	}
	return _parentModel->parseExpression(transition->getGuardExpression()) != 0.0;
}

void ColoredPetriNetNetwork::_removeIncidentArcs(PetriPlace* place) {
	for (auto it = _arcs->list()->begin(); it != _arcs->list()->end();) {
		CPNArc* arc = *it;
		if (arc != nullptr && arc->getPlace() == place) {
			it = _arcs->list()->erase(it);
		} else {
			++it;
		}
	}
}

void ColoredPetriNetNetwork::_removeIncidentArcs(CPNTransition* transition) {
	for (auto it = _arcs->list()->begin(); it != _arcs->list()->end();) {
		CPNArc* arc = *it;
		if (arc != nullptr && arc->getTransition() == transition) {
			it = _arcs->list()->erase(it);
		} else {
			++it;
		}
	}
}
