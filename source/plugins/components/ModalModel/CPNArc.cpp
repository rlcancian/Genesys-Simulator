#include "plugins/data/ModalModel/CPNArc.h"

#include "plugins/components/ModalModel/PetriPlace.h"
#include "plugins/data/ModalModel/CPNTransition.h"

#ifdef PLUGINCONNECT_DYNAMIC
extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &CPNArc::GetPluginInformation;
}
#endif

CPNArc::CPNArc(Model* model, std::string name)
	: ModelDataDefinition(model, Util::TypeOf<CPNArc>(), name) {
}

CPNArc::CPNArc(Model* model, PetriPlace* place, CPNTransition* transition, Direction direction, std::string name)
	: ModelDataDefinition(model, Util::TypeOf<CPNArc>(), name),
	  _place(place),
	  _transition(transition),
	  _direction(direction) {
	_syncEndpointNames();
}

CPNArc::~CPNArc() {
	delete _inscriptions;
}

PluginInformation* CPNArc::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<CPNArc>(), &CPNArc::LoadInstance, &CPNArc::NewInstance);
	info->setCategory("ModalModel");
	info->setDescriptionHelp("Directed bipartite fixed-inscription arc used by ColoredPetriNetNetwork.");
	return info;
}

ModelDataDefinition* CPNArc::LoadInstance(Model* model, PersistenceRecord* fields) {
	CPNArc* arc = new CPNArc(model);
	try {
		arc->_loadInstance(fields);
	} catch (const std::exception& e) {
		arc->traceError("Failed to load CPNArc instance: " + std::string(e.what()));
	}
	return arc;
}

ModelDataDefinition* CPNArc::NewInstance(Model* model, std::string name) {
	return new CPNArc(model, name);
}

void CPNArc::setPlace(PetriPlace* place) {
	_place = place;
	_syncEndpointNames();
}

PetriPlace* CPNArc::getPlace() const {
	return _place;
}

void CPNArc::setTransition(CPNTransition* transition) {
	_transition = transition;
	_syncEndpointNames();
}

CPNTransition* CPNArc::getTransition() const {
	return _transition;
}

void CPNArc::setDirection(Direction direction) {
	_direction = direction;
}

CPNArc::Direction CPNArc::getDirection() const {
	return _direction;
}

bool CPNArc::isInputArc() const {
	return _direction == Direction::PlaceToTransition;
}

bool CPNArc::isOutputArc() const {
	return _direction == Direction::TransitionToPlace;
}

void CPNArc::setInscription(std::string color, unsigned int quantity) {
	if (quantity == 0u) {
		_inscriptions->erase(color);
	} else {
		(*_inscriptions)[color] = quantity;
	}
}

unsigned int CPNArc::getInscription(std::string color) const {
	auto it = _inscriptions->find(color);
	return it != _inscriptions->end() ? it->second : 0u;
}

const std::map<std::string, unsigned int>& CPNArc::getInscriptions() const {
	return *_inscriptions;
}

void CPNArc::clearInscriptions() {
	_inscriptions->clear();
}

std::string CPNArc::getPlaceName() const {
	return _place != nullptr ? _place->getName() : _placeName;
}

std::string CPNArc::getTransitionName() const {
	return _transition != nullptr ? _transition->getName() : _transitionName;
}

std::string CPNArc::show() {
	return ModelDataDefinition::show() +
	       ", place=\"" + getPlaceName() + "\"" +
	       ", transition=\"" + getTransitionName() + "\"" +
	       ", direction=" + std::to_string(static_cast<unsigned int>(_direction)) +
	       ", inscriptions=" + std::to_string(_inscriptions->size());
}

bool CPNArc::_loadInstance(PersistenceRecord* fields) {
	bool res = ModelDataDefinition::_loadInstance(fields);
	if (res) {
		_place = nullptr;
		_transition = nullptr;
		_placeName = fields->loadField("place", std::string(""));
		_transitionName = fields->loadField("transition", std::string(""));
		_direction = static_cast<Direction>(fields->loadField("direction", static_cast<unsigned int>(Direction::PlaceToTransition)));
		_inscriptions->clear();
		unsigned int size = fields->loadField("inscriptions", 0u);
		for (unsigned int i = 0; i < size; i++) {
			const std::string color = fields->loadField("inscriptionColor" + Util::StrIndex(i), std::string("default"));
			const unsigned int quantity = fields->loadField("inscriptionQuantity" + Util::StrIndex(i), 0u);
			if (quantity > 0u) {
				(*_inscriptions)[color] = quantity;
			}
		}
	}
	return res;
}

void CPNArc::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
	ModelDataDefinition::_saveInstance(fields, saveDefaultValues);
	fields->saveField("place", getPlaceName(), std::string(""), saveDefaultValues);
	fields->saveField("transition", getTransitionName(), std::string(""), saveDefaultValues);
	fields->saveField("direction", static_cast<unsigned int>(_direction), static_cast<unsigned int>(Direction::PlaceToTransition), saveDefaultValues);
	fields->saveField("inscriptions", _inscriptions->size(), 0u, saveDefaultValues);
	unsigned int i = 0;
	for (const auto& pair : *_inscriptions) {
		fields->saveField("inscriptionColor" + Util::StrIndex(i), pair.first, std::string("default"), saveDefaultValues);
		fields->saveField("inscriptionQuantity" + Util::StrIndex(i), pair.second, 0u, saveDefaultValues);
		i++;
	}
}

bool CPNArc::_check(std::string& errorMessage) {
	bool resultAll = true;
	if (_place == nullptr) {
		errorMessage += "CPNArc \"" + getName() + "\" requires a place endpoint. ";
		resultAll = false;
	}
	if (_transition == nullptr) {
		errorMessage += "CPNArc \"" + getName() + "\" requires a transition endpoint. ";
		resultAll = false;
	}
	if (_inscriptions->empty()) {
		errorMessage += "CPNArc \"" + getName() + "\" requires at least one fixed inscription. ";
		resultAll = false;
	}
	for (const auto& pair : *_inscriptions) {
		if (pair.first.empty() || pair.second == 0u) {
			errorMessage += "CPNArc \"" + getName() + "\" has an invalid fixed inscription. ";
			resultAll = false;
		}
	}
	return resultAll;
}

void CPNArc::_syncEndpointNames() {
	if (_place != nullptr) {
		_placeName = _place->getName();
	}
	if (_transition != nullptr) {
		_transitionName = _transition->getName();
	}
}
