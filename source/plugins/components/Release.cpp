/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   Release.cpp
 * Author: rafael.luiz.cancian
 *
 * Created on 21 de Agosto de 2018, 16:17
 */

#include "Release.h"
#include "../../kernel/simulator/Model.h"
#include "../../kernel/simulator/Simulator.h"
#include "../data/Resource.h"
//#include "../../kernel/simulator/Attribute.h"
#include <assert.h>
#include <cmath>

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &Release::GetPluginInformation;
}
#endif

ModelDataDefinition* Release::NewInstance(Model* model, std::string name) {
	return new Release(model, name);
}

Release::Release(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<Release>(), name) {
}

std::string Release::show() {
	std::string txt = ModelComponent::show() +
			"priority=" + std::to_string(_priority) +
			"releaseRequests={";
	unsigned short i = 0;
	for (SeizableItem* item : *_releaseRequests->list()) {
		txt += "request" + std::to_string(i++) + "=[" + item->show() + "],";
	}
	txt = txt.substr(0, txt.length() - 1) + "}";
	return txt;
}

void Release::setPriority(unsigned short _priority) {
	this->_priority = _priority;
}

unsigned short Release::priority() const {
	return _priority;
}

List<SeizableItem*>* Release::getReleaseRequests() const {
	return _releaseRequests;
}

//void Release::setResource(Resource* _resource) {
//	this->_resource = _resource;
//}

//Resource* Release::resource() const {
//	return _resource;
//}

Resource* Release::_getResourceFromSeizableItem(SeizableItem* seizable, Entity* entity) {
	Resource* resource;
	if (seizable->getSeizableType() == SeizableItem::SeizableType::RESOURCE) {
		resource = seizable->getResource();
	} else { // assume SET
		SeizableItem::SelectionRule rule = seizable->getSelectionRule();
		Set* set = seizable->getSet();
		unsigned int index = 0;
		double value, bestValue;
		unsigned int bestIndex;
		switch (rule) {
			case SeizableItem::SelectionRule::CYCLICAL:
				index = (seizable->getLastMemberSeized() + 1) % _releaseRequests->list()->size();
				_parentModel->getTracer()->trace("Member index " + std::to_string(index) + " was cyclically choosen", TraceManager::Level::L9_mostDetailed);
				break;
			case SeizableItem::SelectionRule::LARGESTREMAININGCAPACITY:
				unsigned int bestIndex;
				index = 0;
				bestValue = std::numeric_limits<double>::min();
				for (ModelDataDefinition* dd : *seizable->getSet()->getElementSet()->list()) {
					resource = static_cast<Resource*> (dd);
					value = resource->getCapacity() - resource->getNumberBusy();
					if (value > bestValue) {
						bestValue = value;
						bestIndex = index;
					}
					_parentModel->getTracer()->trace("Member index " + std::to_string(index) + " has " + Util::StrTruncIfInt(value) + " remaining capacity. Largest one is " + Util::StrTruncIfInt(bestValue) + " from index " + std::to_string(bestIndex), TraceManager::Level::L9_mostDetailed);
					index++;
				}
				index = bestIndex;
				break;
			case SeizableItem::SelectionRule::RANDOM:
				// @TODO: RANDOM IS REALLY A PROBLEM!!! NOW IT MAY CAUSE AN ERROR (DEQUEUE AN ENTITY BECAUSE IT CAN SEIZE ALL REQUESTS, BUT ANOTHER RANDOM REQUEST MY BE SELECTED AFTER, IT IT MAY BE BUSY
				index = std::trunc(rand() * this->_releaseRequests->list()->size());
				_parentModel->getTracer()->trace("Member index " + std::to_string(index) + " was randomlly choosen", TraceManager::Level::L9_mostDetailed);
				break;
			case SeizableItem::SelectionRule::SMALLESTNUMBERBUSY:
				bestValue = std::numeric_limits<double>::max();
				index = 0;
				for (ModelDataDefinition* dd : *seizable->getSet()->getElementSet()->list()) {
					resource = static_cast<Resource*> (dd);
					value = resource->getNumberBusy();
					if (value < bestValue) {
						bestValue = value;
						bestIndex = index;
					}
					_parentModel->getTracer()->trace("Member index " + std::to_string(index) + " has " + Util::StrTruncIfInt(value) + " number busy. Smallest one is " + Util::StrTruncIfInt(bestValue) + " from index " + std::to_string(bestIndex), TraceManager::Level::L9_mostDetailed);
					index++;
				}
				index = bestIndex;
				break;
			case SeizableItem::SelectionRule::SPECIFICMEMBER:
				index = _parentModel->parseExpression(seizable->getSaveAttribute());
				_parentModel->getTracer()->trace("Member index " + std::to_string(index) + " was specifically choosen", TraceManager::Level::L9_mostDetailed);
				break;
			case SeizableItem::SelectionRule::PREFEREDORDER:
				bestValue = 0;
				index = 0;
				unsigned int quantity = _parentModel->parseExpression(seizable->getQuantityExpression());
				unsigned int numRecAvaliable = 0;
				for (ModelDataDefinition* dd : *seizable->getSet()->getElementSet()->list()) {
					resource = static_cast<Resource*> (dd);
					if ((resource->getCapacity() - resource->getNumberBusy()) >= quantity) { // resource "avaliable"
						numRecAvaliable++;
					}
				}
				unsigned int lastPreferedOrder = seizable->getLastPreferedOrder();
				if (numRecAvaliable == 0) {
					// to avoid the entity always to be waiting on the queue of the resource index 0 when there is no avaliable resource, 
					// it will be waiting on the "preferedOrder"-th index member
					if (++lastPreferedOrder < seizable->getSet()->getElementSet()->size()) {
						index = lastPreferedOrder;
					}
					_parentModel->getTracer()->trace("There is no available resources. Will request the " + std::to_string(index) + "th member index", TraceManager::Level::L9_mostDetailed);
				} else {
					seizable->setLastPreferedOrder(++lastPreferedOrder);
					if (lastPreferedOrder >= numRecAvaliable) {
						seizable->setLastPreferedOrder(1);
						_parentModel->getTracer()->trace("There isn't " + std::to_string(lastPreferedOrder) + " available resources. Preferable order is now the 1th", TraceManager::Level::L9_mostDetailed);
					}
					for (ModelDataDefinition* dd : *seizable->getSet()->getElementSet()->list()) {
						resource = static_cast<Resource*> (dd);
						if (resource->getCapacity() - resource->getNumberBusy() >= quantity) { // resource "avaliable"
							bestValue++; // bestValue is always the i-th available resource index
							if (bestValue <= lastPreferedOrder) { // changed "==" to "<=" so if not enought available resources, entity will not be always on the index=0 resource's queue
								bestIndex = index;
							}
							_parentModel->getTracer()->trace("Member index " + std::to_string(index) + " is the " + Util::StrTruncIfInt(bestValue) + "th available one. Will chosse the  " + Util::StrTruncIfInt(lastPreferedOrder) + "th one.", TraceManager::Level::L9_mostDetailed);
							if (bestValue == lastPreferedOrder) {
								break;
							}
						} else {
							_parentModel->getTracer()->trace("Member index " + std::to_string(index) + " is not available", TraceManager::Level::L9_mostDetailed);
						}
						index++;
					}
					index = bestIndex;
				}
				break;
		}
		_parentModel->getTracer()->trace("Member of set " + set->getName() + " chosen index " + std::to_string(index), TraceManager::Level::L8_detailed);
		resource = static_cast<Resource*> (set->getElementSet()->getAtRank(index));
		assert(resource != nullptr);
	}
	return resource;
}


void Release::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	for (SeizableItem* seizable : *_releaseRequests->list()) {
		Resource* resource = _getResourceFromSeizableItem(seizable, entity);
		unsigned int quantity = _parentModel->parseExpression(seizable->getQuantityExpression());
		assert(resource->getNumberBusy() >= quantity); // 202104 ops. maybe not anymore
		_parentModel->getTracer()->traceSimulation(this, _parentModel->getSimulation()->getSimulatedTime(), entity, this, entity->getName() + " releases " + std::to_string(quantity) + " units of resource \"" + resource->getName() + "\" seized on time " + std::to_string(resource->getLastTimeSeized()));
		resource->release(quantity); //{releases and sets the 'LastTimeSeized'property}
		if (_reportStatistics) {
			double timeSeized = resource->getLastTimeSeized();
			double allocationEntityResource = entity->getAttributeValue("Entity.Allocation."+resource->getName()); //@TODO: Seize is not setting this attribute. Fiz it.
			std::string allocationCategory = Util::StrAllocation(static_cast<Util::AllocationType>((int) allocationEntityResource));
			entity->getEntityType()->addGetStatisticsCollector(entity->getEntityTypeName() + "."+allocationCategory+"Time")->getStatistics()->getCollector()->addValue(timeSeized);
			entity->setAttributeValue("Entity.Total"+allocationCategory+"Time", entity->getAttributeValue("Entity.Total"+allocationCategory+"Time") + timeSeized, true);			
		}
	}
	_parentModel->sendEntityToComponent(entity, this->getConnections()->getFrontConnection());
}

void Release::_initBetweenReplications() {
	//for (SeizableItem* seizable : *_releaseRequests->list()) {
	//	if (seizable->getSeizableType() == SeizableItem::SeizableType::RESOURCE)
	//		seizable->getResource()->initBetweenReplications();
	//	else if (seizable->getSeizableType() == SeizableItem::SeizableType::SET)
	//		seizable->getSet()->initBetweenReplications();
	//}
}

bool Release::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		this->_priority = fields->loadField("priority", DEFAULT.priority);
		unsigned short numRequests = fields->loadField("ReleaseRequests", DEFAULT.releaseRequestSize);
		for (unsigned short i = 0; i < numRequests; i++) {
			SeizableItem* item = new SeizableItem(nullptr, "", SeizableItem::SelectionRule::LARGESTREMAININGCAPACITY);
			item->setElementManager(_parentModel->getDataManager());
			item->loadInstance(fields, i);
			this->_releaseRequests->insert(item);
		}
	}
	return res;
}

void Release::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("Priority", _priority, DEFAULT.priority, saveDefaultValues);
	fields->saveField("ReleaseRequests", _releaseRequests->size(), DEFAULT.releaseRequestSize, saveDefaultValues);
	unsigned short i = 0;
	for (SeizableItem* request : *_releaseRequests->list()) {
		request->saveInstance(fields, i, saveDefaultValues);
		i++;
	}
}

void Release::_createInternalAndAttachedData() {
	int i = 0;
	for (SeizableItem* seizable : * _releaseRequests->list()) {
		if (seizable->getSeizableType() == SeizableItem::SeizableType::RESOURCE) {
			Resource* resource = seizable->getResource();
			if (resource == nullptr && _parentModel->isAutomaticallyCreatesModelDataDefinitions()) {
				resource = _parentModel->getParentSimulator()->getPlugins()->newInstance<Resource>(_parentModel);
			}
			_attachedDataInsert("SeizableItem" + Util::StrIndex(i), resource);
		} else if (seizable->getSeizableType() == SeizableItem::SeizableType::SET) {
			Set* set = seizable->getSet();
			if (set == nullptr && _parentModel->isAutomaticallyCreatesModelDataDefinitions()) {
				set = _parentModel->getParentSimulator()->getPlugins()->newInstance<Set>(_parentModel);
			}
			_attachedDataInsert("SeizableItem" + Util::StrIndex(i), set);
		}
		i++;
	}
}

bool Release::_check(std::string* errorMessage) {
	bool resultAll = true;
	int i = 0;
	for (SeizableItem* seizable : * _releaseRequests->list()) {
		resultAll &= _parentModel->checkExpression(seizable->getQuantityExpression(), "quantity", errorMessage);
		if (seizable->getSeizableType() == SeizableItem::SeizableType::RESOURCE) {
			// Resource* resource = seizable->getResource();
			resultAll &= _parentModel->getDataManager()->check(Util::TypeOf<Resource>(), seizable->getResource(), "Resource", errorMessage);
		} else if (seizable->getSeizableType() == SeizableItem::SeizableType::SET) {
			// Set* set = seizable->getSet();
			resultAll &= _parentModel->getDataManager()->check(Util::TypeOf<Set>(), seizable->getSet(), "Set", errorMessage);
		}
		i++;
	}
	return resultAll;
}

PluginInformation* Release::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Release>(), &Release::LoadInstance, &Release::NewInstance);
	info->insertDynamicLibFileDependence("resource.so");
	std::string help = "The Release module is used to release units of a resource that an entity previously has seized.";
	help += " This module may be used to release individual resources or may be used to release resources within a set.";
	help += " For each resource to be released, the name and quantity to release are specified.";
	help += " When the entity enters the Release module, it gives up control of the specified resource(s).";
	help += " Any entities waiting in queues for those resources will gain control of the resources immediately.";
	help += " TYPICAL USES: (1) Finishing a customer order (release the operator); (2) Completing a tax return (release the accountant);";
	help += " (3) Leaving the hospital (release the doctor, nurse, hospital room)";
	info->setDescriptionHelp(help);
	return info;
}

ModelComponent* Release::LoadInstance(Model* model, PersistenceRecord *fields) {
	Release* newComponent = new Release(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newComponent;

}
