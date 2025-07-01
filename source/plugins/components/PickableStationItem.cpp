/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/class.cc to edit this template
 */

/*
 * File:   PickableStationItem.cpp
 * Author: rlcancian
 *
 * Created on 1 de dezembro de 2022, 14:00
 */

#include "PickableStationItem.h"
#include "../../kernel/simulator/Simulator.h"

PickableStationItem::PickableStationItem(Model* model, std::string stationName, std::string queueName, std::string resourceName) {
    ModelDataDefinition* station = model->getDataManager()->getDataDefinition(Util::TypeOf<Station>(), stationName);
    if (station == nullptr) {
        station = model->getParentSimulator()->getPluginManager()->newInstance<Station>(model, stationName);
    }
    ModelDataDefinition* resource = model->getDataManager()->getDataDefinition(Util::TypeOf<Resource>(), resourceName);
    if (resource == nullptr) {
        resource = model->getParentSimulator()->getPluginManager()->newInstance<Resource>(model, resourceName);
    }
    ModelDataDefinition* queue = model->getDataManager()->getDataDefinition(Util::TypeOf<Queue>(), queueName);
    if (queue == nullptr) {
        queue = model->getParentSimulator()->getPluginManager()->newInstance<Queue>(model, queueName);
    }
    if (queue != nullptr) {
        PickableStationItem(dynamic_cast<Station*>(station), dynamic_cast<Queue*>(queue));
    } else if (resource != nullptr) {
        PickableStationItem(dynamic_cast<Station*>(station), dynamic_cast<Resource*>(resource));
    }

    _stationName = stationName;

    SimulationControlGeneric<std::string>* propExpression = new SimulationControlGeneric<std::string>(
                                    std::bind(&PickableStationItem::getExpression, this), std::bind(&PickableStationItem::setExpression, this, std::placeholders::_1),
                                    Util::TypeOf<PickableStationItem>(), "", "Expression", "");
    SimulationControlGenericClass<Queue*, Model*, Queue>* propQueue = new SimulationControlGenericClass<Queue*, Model*, Queue>(
                                    model,
                                    std::bind(&PickableStationItem::getQueue, this), std::bind(&PickableStationItem::setQueue, this, std::placeholders::_1),
                                    Util::TypeOf<PickableStationItem>(), "", "Queue", "");
    SimulationControlGenericClass<Resource*, Model*, Resource>* propResource = new SimulationControlGenericClass<Resource*, Model*, Resource>(
                                    model,
                                    std::bind(&PickableStationItem::getResource, this), std::bind(&PickableStationItem::setResource, this, std::placeholders::_1),
                                    Util::TypeOf<PickableStationItem>(), "", "Resource", "");
    SimulationControlGenericClass<Station*, Model*, Station>* propStation = new SimulationControlGenericClass<Station*, Model*, Station>(
                                    model,
                                    std::bind(&PickableStationItem::getStation, this), std::bind(&PickableStationItem::setStation, this, std::placeholders::_1),
                                    Util::TypeOf<PickableStationItem>(), "", "Station", "");

    model->getControls()->insert(propExpression);
    model->getControls()->insert(propQueue);
    model->getControls()->insert(propResource);
    model->getControls()->insert(propStation);

    // setting properties
    _addProperty(propExpression);
    _addProperty(propQueue);
    _addProperty(propResource);
    _addProperty(propStation);
}

PickableStationItem::PickableStationItem(Model* model, std::string stationName, std::string expression) {
    ModelDataDefinition* station = model->getDataManager()->getDataDefinition(Util::TypeOf<Station>(), stationName);
    if (station == nullptr) {
        station = model->getParentSimulator()->getPluginManager()->newInstance<Station>(model, stationName);
    }
    PickableStationItem(dynamic_cast<Station*>(station),expression);
}

PickableStationItem::PickableStationItem(Station* station, Queue* queue) {
    _station = station;
    _queue = queue;
}

PickableStationItem::PickableStationItem(Station* station, Resource* resource) {
    _station = station;
    _resource = resource;
}

PickableStationItem::PickableStationItem(Station* station, std::string expression) {
    _station = station;
    _expression = expression;
}

std::string PickableStationItem::getName() {
    return _stationName;
}

std::string PickableStationItem::getExpression() const {
    return _expression;
}

Queue* PickableStationItem::getQueue() const {
    return _queue;
}

Resource* PickableStationItem::getResource() const {
    return _resource;
}

Station* PickableStationItem::getStation() const {
    return _station;
}

void PickableStationItem::setExpression(std::string _expression) {
    this->_expression = _expression;
}

void PickableStationItem::setQueue(Queue* _queue) {
    this->_queue = _queue;
}

void PickableStationItem::setResource(Resource* _resource) {
    this->_resource = _resource;
}

void PickableStationItem::setStation(Station* _station) {
    this->_station = _station;
}

void PickableStationItem::_addProperty(PropertyBase* property) {
    _properties->insert(property);
}

List<PropertyBase*>* PickableStationItem::getProperties() const {
    return _properties;
}

