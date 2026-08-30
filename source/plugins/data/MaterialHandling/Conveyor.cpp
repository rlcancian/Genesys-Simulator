#include "plugins/data/MaterialHandling/Conveyor.h"
#include "../../../kernel/simulator/model/Model.h"
#include "kernel/simulator/Simulator.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &Conveyor::GetPluginInformation;
}
#endif

ModelDataDefinition* Conveyor::NewInstance(Model* model, std::string name) {
	return new Conveyor(model, name);
}

Conveyor::Conveyor(Model* model, std::string name) : ModelDataDefinition(model, Util::TypeOf<Conveyor>(), name) {
	auto* propSegment = new SimulationControlGenericClass<Segment*, Model*, Segment>(
			_parentModel,
			std::bind(&Conveyor::getSegment, this),
			std::bind(&Conveyor::setSegment, this, std::placeholders::_1),
			Util::TypeOf<Conveyor>(), getName(), "Segment", "");
	auto* propVelocity = new SimulationControlDouble(
			std::bind(&Conveyor::getVelocity, this),
			std::bind(&Conveyor::setVelocity, this, std::placeholders::_1),
			Util::TypeOf<Conveyor>(), getName(), "Velocity");
	auto* propCapacity = new SimulationControlDouble(
			std::bind(&Conveyor::getCapacity, this),
			std::bind(&Conveyor::setCapacity, this, std::placeholders::_1),
			Util::TypeOf<Conveyor>(), getName(), "Capacity");
	auto* propInitiallyActive = new SimulationControlBool(
			std::bind(&Conveyor::isInitiallyActive, this),
			std::bind(&Conveyor::setInitiallyActive, this, std::placeholders::_1),
			Util::TypeOf<Conveyor>(), getName(), "InitiallyActive");
	_parentModel->getControls()->insert(propSegment);
	_parentModel->getControls()->insert(propVelocity);
	_parentModel->getControls()->insert(propCapacity);
	_parentModel->getControls()->insert(propInitiallyActive);
	_addSimulationControl(propSegment);
	_addSimulationControl(propVelocity);
	_addSimulationControl(propCapacity);
	_addSimulationControl(propInitiallyActive);
}

std::string Conveyor::show() {
	return ModelDataDefinition::show() +
			", segment=\"" + (_segment != nullptr ? _segment->getName() : std::string()) + "\"" +
			", velocity=" + Util::StrTruncIfInt(std::to_string(_velocity)) +
			", capacity=" + std::to_string(_capacity) +
			", initiallyActive=" + std::string(_initiallyActive ? "true" : "false") +
			", active=" + std::string(_active ? "true" : "false") +
			", currentAllocation=" + std::to_string(_currentAllocation);
}

void Conveyor::setSegment(Segment* segment) {
	_segment = segment;
}

Segment* Conveyor::getSegment() const {
	return _segment;
}

void Conveyor::setVelocity(double velocity) {
	_velocity = velocity;
}

double Conveyor::getVelocity() const {
	return _velocity;
}

void Conveyor::setCapacity(unsigned int capacity) {
	_capacity = capacity;
}

unsigned int Conveyor::getCapacity() const {
	return _capacity;
}

void Conveyor::setInitiallyActive(bool initiallyActive) {
	_initiallyActive = initiallyActive;
	_active = initiallyActive;
}

bool Conveyor::isInitiallyActive() const {
	return _initiallyActive;
}

void Conveyor::setActive(bool active) {
	_active = active;
}

bool Conveyor::isActive() const {
	return _active;
}

unsigned int Conveyor::getCurrentAllocation() const {
	return _currentAllocation;
}

bool Conveyor::access(unsigned int quantity) {
	if (!_active || quantity == 0 || _currentAllocation + quantity > _capacity) {
		return false;
	}
	_currentAllocation += quantity;
	return true;
}

bool Conveyor::exit(unsigned int quantity) {
	if (quantity == 0 || quantity > _currentAllocation) {
		return false;
	}
	_currentAllocation -= quantity;
	return true;
}

double Conveyor::getDistanceBetween(const std::string& fromStationName, const std::string& toStationName) const {
	return _segment != nullptr ? _segment->getDistanceBetween(fromStationName, toStationName) : -1.0;
}

PluginInformation* Conveyor::GetPluginInformation() {
	auto* info = new PluginInformation(Util::TypeOf<Conveyor>(), &Conveyor::LoadInstance, &Conveyor::NewInstance);
	info->setCategory("MaterialHandling");
	info->insertDynamicLibFileDependence("segment.so");
	info->setDescriptionHelp("Defines one minimal conveyor path backed by a Segment, with active state, velocity and simplified concurrent allocation capacity.");
	return info;
}

ModelDataDefinition* Conveyor::LoadInstance(Model* model, PersistenceRecord* fields) {
	auto* newElement = new Conveyor(model);
	try {
		newElement->_loadInstance(fields);
	} catch (const std::exception& e) {
		newElement->traceError("Failed to load Conveyor instance: " + std::string(e.what()));
	}
	return newElement;
}

bool Conveyor::_loadInstance(PersistenceRecord* fields) {
	bool res = ModelDataDefinition::_loadInstance(fields);
	if (res) {
		_velocity = fields->loadField("velocity", DEFAULT.velocity);
		_capacity = fields->loadField("capacity", DEFAULT.capacity);
		_initiallyActive = fields->loadField("initiallyActive", static_cast<int>(DEFAULT.initiallyActive)) != 0;
		_active = _initiallyActive;
		const std::string segmentName = fields->loadField("segment", std::string(""));
		_segment = dynamic_cast<Segment*>(_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<Segment>(), segmentName));
	}
	return res;
}

void Conveyor::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
	ModelDataDefinition::_saveInstance(fields, saveDefaultValues);
	fields->saveField("velocity", _velocity, DEFAULT.velocity, saveDefaultValues);
	fields->saveField("capacity", _capacity, DEFAULT.capacity, saveDefaultValues);
	fields->saveField("initiallyActive", static_cast<int>(_initiallyActive), static_cast<int>(DEFAULT.initiallyActive), saveDefaultValues);
	fields->saveField("segment", _segment != nullptr ? _segment->getName() : std::string(""), std::string(""), saveDefaultValues);
}

bool Conveyor::_check(std::string& errorMessage) {
	bool resultAll = true;
	resultAll &= _parentModel->getDataManager()->check(Util::TypeOf<Segment>(), _segment, "Segment", errorMessage);
	if (_velocity <= 0.0) {
		errorMessage += "Conveyor \"" + getName() + "\" must define velocity > 0. ";
		resultAll = false;
	}
	if (_capacity == 0) {
		errorMessage += "Conveyor \"" + getName() + "\" must define capacity > 0. ";
		resultAll = false;
	}
	return resultAll;
}

void Conveyor::_initBetweenReplications() {
	ModelDataDefinition::_initBetweenReplications();
	_active = _initiallyActive;
	_currentAllocation = 0;
}

void Conveyor::_createEditableDataDefinitions() {
	if (_segment == nullptr) {
		_segment = _parentModel->getParentSimulator()->getPluginManager()->newInstance<Segment>(_parentModel);
	}
	if (_segment != nullptr) {
		_optionalEditableDataDefinitionInsert("Segment", _segment);
	} else {
		_optionalEditableDataDefinitionRemove("Segment");
	}
}
