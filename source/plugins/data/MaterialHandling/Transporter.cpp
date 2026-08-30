#include "plugins/data/MaterialHandling/Transporter.h"
#include "../../../kernel/simulator/model/Model.h"
#include "kernel/simulator/Simulator.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &Transporter::GetPluginInformation;
}
#endif

ModelDataDefinition* Transporter::NewInstance(Model* model, std::string name) {
	return new Transporter(model, name);
}

Transporter::Transporter(Model* model, std::string name) : ModelDataDefinition(model, Util::TypeOf<Transporter>(), name) {
	auto* propDistance = new SimulationControlGenericClass<Distance*, Model*, Distance>(
			_parentModel,
			std::bind(&Transporter::getDistance, this),
			std::bind(&Transporter::setDistance, this, std::placeholders::_1),
			Util::TypeOf<Transporter>(), getName(), "Distance", "");
	auto* propInitialStation = new SimulationControlGenericClass<Station*, Model*, Station>(
			_parentModel,
			std::bind(&Transporter::getInitialStation, this),
			std::bind(&Transporter::setInitialStation, this, std::placeholders::_1),
			Util::TypeOf<Transporter>(), getName(), "InitialStation", "");
	auto* propSpeed = new SimulationControlDouble(
			std::bind(&Transporter::getSpeed, this),
			std::bind(&Transporter::setSpeed, this, std::placeholders::_1),
			Util::TypeOf<Transporter>(), getName(), "Speed");
	auto* propInitiallyActive = new SimulationControlBool(
			std::bind(&Transporter::isInitiallyActive, this),
			std::bind(&Transporter::setInitiallyActive, this, std::placeholders::_1),
			Util::TypeOf<Transporter>(), getName(), "InitiallyActive");
	_parentModel->getControls()->insert(propDistance);
	_parentModel->getControls()->insert(propInitialStation);
	_parentModel->getControls()->insert(propSpeed);
	_parentModel->getControls()->insert(propInitiallyActive);
	_addSimulationControl(propDistance);
	_addSimulationControl(propInitialStation);
	_addSimulationControl(propSpeed);
	_addSimulationControl(propInitiallyActive);
}

std::string Transporter::show() {
	return ModelDataDefinition::show() +
			", distance=\"" + (_distance != nullptr ? _distance->getName() : std::string()) + "\"" +
			", initialStation=\"" + (_initialStation != nullptr ? _initialStation->getName() : std::string()) + "\"" +
			", currentStation=\"" + (_currentStation != nullptr ? _currentStation->getName() : std::string()) + "\"" +
			", speed=" + Util::StrTruncIfInt(std::to_string(_speed)) +
			", initiallyActive=" + std::string(_initiallyActive ? "true" : "false") +
			", active=" + std::string(_active ? "true" : "false") +
			", busy=" + std::string(_busy ? "true" : "false");
}

void Transporter::setDistance(Distance* distance) {
	_distance = distance;
}

Distance* Transporter::getDistance() const {
	return _distance;
}

void Transporter::setInitialStation(Station* station) {
	_initialStation = station;
	_currentStation = station;
}

Station* Transporter::getInitialStation() const {
	return _initialStation;
}

Station* Transporter::getCurrentStation() const {
	return _currentStation;
}

void Transporter::setSpeed(double speed) {
	_speed = speed;
}

double Transporter::getSpeed() const {
	return _speed;
}

void Transporter::setInitiallyActive(bool initiallyActive) {
	_initiallyActive = initiallyActive;
	_active = initiallyActive;
}

bool Transporter::isInitiallyActive() const {
	return _initiallyActive;
}

void Transporter::setActive(bool active) {
	_active = active;
}

bool Transporter::isActive() const {
	return _active;
}

bool Transporter::isBusy() const {
	return _busy;
}

bool Transporter::reserve() {
	if (!_active || _busy) {
		return false;
	}
	_busy = true;
	return true;
}

void Transporter::releaseAt(Station* station) {
	_currentStation = station;
	_busy = false;
}

double Transporter::getTravelTime(const Station* fromStation, const Station* toStation) const {
	if (_distance == nullptr || fromStation == nullptr || toStation == nullptr || _speed <= 0.0) {
		return -1.0;
	}
	const double distance = _distance->getDistanceBetween(fromStation->getName(), toStation->getName());
	if (distance < 0.0) {
		return -1.0;
	}
	return distance / _speed;
}

void Transporter::handleTravelCompletion(void* parameter) {
	auto* completion = static_cast<TravelCompletion*>(parameter);
	releaseAt(completion != nullptr ? completion->destination : nullptr);
	delete completion;
}

PluginInformation* Transporter::GetPluginInformation() {
	auto* info = new PluginInformation(Util::TypeOf<Transporter>(), &Transporter::LoadInstance, &Transporter::NewInstance);
	info->setCategory("MaterialHandling");
	info->insertDynamicLibFileDependence("distance.so");
	info->insertDynamicLibFileDependence("station.so");
	info->setDescriptionHelp("Defines one minimal free-path transporter backed by a Distance set, current station, active state and single-unit reservation.");
	return info;
}

ModelDataDefinition* Transporter::LoadInstance(Model* model, PersistenceRecord* fields) {
	auto* newElement = new Transporter(model);
	try {
		newElement->_loadInstance(fields);
	} catch (const std::exception& e) {
		newElement->traceError("Failed to load Transporter instance: " + std::string(e.what()));
	}
	return newElement;
}

bool Transporter::_loadInstance(PersistenceRecord* fields) {
	bool res = ModelDataDefinition::_loadInstance(fields);
	if (res) {
		_speed = fields->loadField("speed", DEFAULT.speed);
		_initiallyActive = fields->loadField("initiallyActive", static_cast<int>(DEFAULT.initiallyActive)) != 0;
		_active = _initiallyActive;
		const std::string distanceName = fields->loadField("distance", std::string(""));
		_distance = dynamic_cast<Distance*>(_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<Distance>(), distanceName));
		const std::string stationName = fields->loadField("initialStation", std::string(""));
		_initialStation = dynamic_cast<Station*>(_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<Station>(), stationName));
		_currentStation = _initialStation;
		_busy = false;
	}
	return res;
}

void Transporter::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
	ModelDataDefinition::_saveInstance(fields, saveDefaultValues);
	fields->saveField("speed", _speed, DEFAULT.speed, saveDefaultValues);
	fields->saveField("initiallyActive", static_cast<int>(_initiallyActive), static_cast<int>(DEFAULT.initiallyActive), saveDefaultValues);
	fields->saveField("distance", _distance != nullptr ? _distance->getName() : std::string(""), std::string(""), saveDefaultValues);
	fields->saveField("initialStation", _initialStation != nullptr ? _initialStation->getName() : std::string(""), std::string(""), saveDefaultValues);
}

bool Transporter::_check(std::string& errorMessage) {
	bool resultAll = true;
	resultAll &= _parentModel->getDataManager()->check(Util::TypeOf<Distance>(), _distance, "Distance", errorMessage);
	resultAll &= _parentModel->getDataManager()->check(Util::TypeOf<Station>(), _initialStation, "InitialStation", errorMessage);
	if (_speed <= 0.0) {
		errorMessage += "Transporter \"" + getName() + "\" must define speed > 0. ";
		resultAll = false;
	}
	return resultAll;
}

void Transporter::_initBetweenReplications() {
	ModelDataDefinition::_initBetweenReplications();
	_currentStation = _initialStation;
	_active = _initiallyActive;
	_busy = false;
}

void Transporter::_createEditableDataDefinitions() {
	if (_distance == nullptr) {
		_distance = _parentModel->getParentSimulator()->getPluginManager()->newInstance<Distance>(_parentModel);
	}
	if (_initialStation == nullptr) {
		_initialStation = _parentModel->getParentSimulator()->getPluginManager()->newInstance<Station>(_parentModel);
		_currentStation = _initialStation;
	}
	if (_distance != nullptr) {
		_optionalEditableDataDefinitionInsert("Distance", _distance);
	} else {
		_optionalEditableDataDefinitionRemove("Distance");
	}
	if (_initialStation != nullptr) {
		_optionalEditableDataDefinitionInsert("InitialStation", _initialStation);
	} else {
		_optionalEditableDataDefinitionRemove("InitialStation");
	}
}
