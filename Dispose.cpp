/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Dispose.cpp
 * Author: rafael.luiz.cancian
 * 
 * Created on 21 de Junho de 2018, 20:13
 */

#include "Dispose.h"
#include "Model.h"

Dispose::Dispose(Model* model, std::string name) : SinkModelComponent(model, Util::TypeOf<Dispose>(), name) {
	//_numberOut = new Counter(_parentModel, getName() + "." + "Count_number_out", this);
	_connections->setMinOutputConnections(0);
	_connections->setMaxOutputConnections(0);
}

std::string Dispose::show() {
	return SinkModelComponent::show();
}

void Dispose::_execute(Entity* entity) {
	if (_reportStatistics) {
		_numberOut->incCountValue();
		if (entity->getEntityType()->isReportStatistics()) {
			double timeInSystem = _parentModel->getSimulation()->getSimulatedTime() - entity->getAttributeValue("Entity.ArrivalTime");
			entity->getEntityType()->addGetStatisticsCollector(entity->getEntityTypeName() + "." + "TotalTimeInSystem")->getStatistics()->getCollector()->addValue(timeInSystem);
		}
	}
	_parentModel->removeEntity(entity, _reportStatistics);
}

bool Dispose::_loadInstance(std::map<std::string, std::string>* fields) {
	return ModelComponent::_loadInstance(fields);
}

void Dispose::_initBetweenReplications() {
	SinkModelComponent::_initBetweenReplications();
}

std::map<std::string, std::string>* Dispose::_saveInstance() {
	std::map<std::string, std::string>* fields = ModelComponent::_saveInstance();
	return fields;

}

bool Dispose::_check(std::string* errorMessage) {
	//
	return true;
}

void Dispose::_createInternalElements() {
	if (_reportStatistics && _numberOut == nullptr) {
		// creates the counter (and then the CStats)
		_numberOut = new Counter(_parentModel, getName() + "." + "CountNumberIn", this);
		_childrenElements->insert({"CountNumberIn", _numberOut});
		// include StatisticsCollector needed for each EntityType
		std::list<ModelElement*>* enttypes = _parentModel->getElements()->getElementList(Util::TypeOf<EntityType>())->list();
		for (std::list<ModelElement*>::iterator it = enttypes->begin(); it != enttypes->end(); it++) {
			if ((*it)->isReportStatistics())
				static_cast<EntityType*> ((*it))->addGetStatisticsCollector((*it)->getName() + "." + "TotalTimeInSystem"); // force create this CStat before model checking
		}
	} else if (!_reportStatistics && _numberOut != nullptr) {
		//_numberOut->~Counter();
		//_numberOut = nullptr;
		// \todo: delete the CSTATS?
		_removeChildrenElements();
	}
}

PluginInformation* Dispose::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Dispose>(), &Dispose::LoadInstance);
	info->setSink(true);
	return info;
}

ModelComponent* Dispose::LoadInstance(Model* model, std::map<std::string, std::string>* fields) {
	Dispose* newComponent = new Dispose(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newComponent;
}