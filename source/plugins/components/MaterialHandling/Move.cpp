#include "plugins/components/MaterialHandling/Move.h"
#include "../../../kernel/simulator/model/Model.h"
#include "kernel/simulator/Simulator.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &Move::GetPluginInformation;
}
#endif

ModelDataDefinition* Move::NewInstance(Model* model, std::string name) {
	return new Move(model, name);
}

Move::Move(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<Move>(), name) {
	auto* propTransporter = new SimulationControlGenericClass<Transporter*, Model*, Transporter>(
			_parentModel,
			std::bind(&Move::getTransporter, this),
			std::bind(&Move::setTransporter, this, std::placeholders::_1),
			Util::TypeOf<Move>(), getName(), "Transporter", "");
	auto* propStation = new SimulationControlGenericClass<Station*, Model*, Station>(
			_parentModel,
			std::bind(&Move::getStation, this),
			std::bind(&Move::setStation, this, std::placeholders::_1),
			Util::TypeOf<Move>(), getName(), "Station", "");
	auto* propStationExpression = new SimulationControlGeneric<std::string>(
			std::bind(&Move::getStationExpression, this),
			std::bind(&Move::setStationExpression, this, std::placeholders::_1),
			Util::TypeOf<Move>(), getName(), "StationExpression", "");
	_parentModel->getControls()->insert(propTransporter);
	_parentModel->getControls()->insert(propStation);
	_parentModel->getControls()->insert(propStationExpression);
	_addSimulationControl(propTransporter);
	_addSimulationControl(propStation);
	_addSimulationControl(propStationExpression);
}

std::string Move::show() {
	return ModelComponent::show() +
			", transporter=\"" + (_transporter != nullptr ? _transporter->getName() : std::string()) + "\"" +
			", station=\"" + (_station != nullptr ? _station->getName() : std::string()) + "\"" +
			", stationExpression=\"" + _stationExpression + "\"";
}

ModelComponent* Move::LoadInstance(Model* model, PersistenceRecord* fields) {
	auto* newComponent = new Move(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {
		newComponent->traceError("Failed to load Move instance: " + std::string(e.what()));
	}
	return newComponent;
}

void Move::setTransporter(Transporter* transporter) {
	_transporter = transporter;
}

Transporter* Move::getTransporter() const {
	return _transporter;
}

void Move::setStation(Station* station) {
	_station = station;
}

Station* Move::getStation() const {
	return _station;
}

void Move::setStationExpression(std::string stationExpression) {
	_stationExpression = stationExpression;
}

std::string Move::getStationExpression() const {
	return _stationExpression;
}

Station* Move::_resolveDestination() const {
	if (!_stationExpression.empty()) {
		const Util::identification stationId = _parentModel->parseExpression(_stationExpression);
		return dynamic_cast<Station*>(_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<Station>(), stationId));
	}
	return _station;
}

Station* Move::_resolveSource(Entity* entity) const {
	const Util::identification sourceStationId = static_cast<Util::identification>(entity->getAttributeValue("Entity.Station"));
	if (sourceStationId != 0) {
		return dynamic_cast<Station*>(_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<Station>(), sourceStationId));
	}
	return _transporter != nullptr ? _transporter->getCurrentStation() : nullptr;
}

void Move::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	(void)inputPortNumber;
	if (_transporter == nullptr) {
		traceError("Move \"" + getName() + "\" has no Transporter configured");
		return;
	}
	Station* destination = _resolveDestination();
	if (destination == nullptr || destination->getEnterIntoStationComponent() == nullptr) {
		traceError("Move \"" + getName() + "\" could not resolve a destination Station with a matching Enter component");
		return;
	}
	Station* source = _resolveSource(entity);
	if (source == nullptr) {
		traceError("Move \"" + getName() + "\" could not resolve the entity source Station");
		return;
	}
	if (!_transporter->reserve()) {
		traceError("Move \"" + getName() + "\" could not reserve Transporter \"" + _transporter->getName() + "\"");
		return;
	}
	const double travelTime = _transporter->getTravelTime(source, destination);
	if (travelTime < 0.0) {
		_transporter->releaseAt(source);
		traceError("Move \"" + getName() + "\" could not compute a valid transport time from \"" + source->getName() + "\" to \"" + destination->getName() + "\"");
		return;
	}
	if (_reportStatistics && _numberIn != nullptr) {
		_numberIn->incCountValue();
	}
	if (entity->getEntityType() != nullptr && entity->getEntityType()->isReportStatistics()) {
		entity->getEntityType()->addGetStatisticsCollector(entity->getEntityTypeName() + ".TransferTime")->getStatistics()->getCollector()->addValue(travelTime);
		entity->setAttributeValue("Entity.TotalTransferTime", entity->getAttributeValue("Entity.TotalTransferTime") + travelTime, "", true);
	}
	source->leave(entity);
	entity->setAttributeValue("Entity.Station", destination->getId(), "", true);
	if (travelTime > 0.0) {
		const double arrivalTime = _parentModel->getSimulation()->getSimulatedTime() + travelTime;
		auto* completion = new Transporter::TravelCompletion();
		completion->destination = destination;
		auto* internalEvent = new InternalEvent(arrivalTime, "TransporterTravelComplete");
		internalEvent->setEventHandler(_transporter, &Transporter::handleTravelCompletion, completion);
		_parentModel->getFutureEvents()->insert(internalEvent);
		_parentModel->getFutureEvents()->insert(new Event(arrivalTime, entity, destination->getEnterIntoStationComponent()));
	} else {
		_transporter->releaseAt(destination);
		_parentModel->sendEntityToComponent(entity, destination->getEnterIntoStationComponent(), 0.0);
	}
}

bool Move::_loadInstance(PersistenceRecord* fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		_stationExpression = fields->loadField("stationExpression", DEFAULT.stationExpression);
		_transporter = dynamic_cast<Transporter*>(_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<Transporter>(), fields->loadField("transporter", std::string(""))));
		_station = dynamic_cast<Station*>(_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<Station>(), fields->loadField("station", std::string(""))));
	}
	return res;
}

void Move::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("stationExpression", _stationExpression, DEFAULT.stationExpression, saveDefaultValues);
	fields->saveField("transporter", _transporter != nullptr ? _transporter->getName() : std::string(""), std::string(""), saveDefaultValues);
	fields->saveField("station", _station != nullptr ? _station->getName() : std::string(""), std::string(""), saveDefaultValues);
}

bool Move::_check(std::string& errorMessage) {
	std::list<ModelDataDefinition*>* enttypes = _parentModel->getDataManager()->getDataDefinitionList(Util::TypeOf<EntityType>())->list();
	for (ModelDataDefinition* modeldatum : *enttypes) {
		if (modeldatum->isReportStatistics()) {
			static_cast<EntityType*>(modeldatum)->addGetStatisticsCollector(modeldatum->getName() + ".TransferTime");
		}
	}
	bool resultAll = true;
	resultAll &= _parentModel->getDataManager()->check(Util::TypeOf<Transporter>(), _transporter, "Transporter", errorMessage);
	if (_stationExpression.empty()) {
		resultAll &= _parentModel->getDataManager()->check(Util::TypeOf<Station>(), _station, "Station", errorMessage);
		if (_station != nullptr && _station->getEnterIntoStationComponent() == nullptr) {
			errorMessage += "Move destination Station has no Enter component. ";
			resultAll = false;
		}
	} else {
		resultAll &= _parentModel->checkExpression(_stationExpression, "StationExpression", errorMessage);
	}
	if (this->getConnectionManager()->size() != 0) {
		errorMessage += "Move \"" + getName() + "\" transfers directly to a Station and should not declare ordinary output connections. ";
		resultAll = false;
	}
	return resultAll;
}

void Move::_createInternalStatisticReporters() {
	if (_reportStatistics) {
		if (_numberIn == nullptr) {
			_numberIn = new Counter(_parentModel, getName() + ".CountNumberIn", this);
		}
		if (_numberIn != nullptr) {
			_statisticReporterInsert("CountNumberIn", _numberIn);
		}
	} else {
		_statisticReportersClear();
		_numberIn = nullptr;
	}
}

void Move::_createEditableDataDefinitions() {
	if (_transporter == nullptr) {
		_transporter = _parentModel->getParentSimulator()->getPluginManager()->newInstance<Transporter>(_parentModel);
	}
	if (_station == nullptr && _stationExpression.empty()) {
		_station = _parentModel->getParentSimulator()->getPluginManager()->newInstance<Station>(_parentModel);
	}
	if (_transporter != nullptr) {
		_optionalEditableDataDefinitionInsert("Transporter", _transporter);
	} else {
		_optionalEditableDataDefinitionRemove("Transporter");
	}
	if (_station != nullptr) {
		_optionalEditableDataDefinitionInsert("Station", _station);
	} else {
		_optionalEditableDataDefinitionRemove("Station");
	}
}

void Move::_createAttachedAttributes() {
	_attachedAttributesInsert({"Entity.TotalTransferTime", "Entity.Station"});
}

PluginInformation* Move::GetPluginInformation() {
	auto* info = new PluginInformation(Util::TypeOf<Move>(), &Move::LoadInstance, &Move::NewInstance);
	info->setCategory("MaterialHandling");
	info->setSendTransfer(true);
	info->insertDynamicLibFileDependence("transporter.so");
	info->insertDynamicLibFileDependence("distance.so");
	info->insertDynamicLibFileDependence("station.so");
	info->setDescriptionHelp("Moves one entity between stations by atomically reserving a minimal free-path Transporter, delaying according to Distance/Speed, and freeing the transporter on arrival.");
	return info;
}
