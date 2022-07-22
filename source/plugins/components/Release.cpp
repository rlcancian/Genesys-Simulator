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

void Release::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	for (SeizableItem* seizable : *_releaseRequests->list()) {
		Resource* resource;
		if (seizable->getSeizableType() == SeizableItem::SeizableType::RESOURCE) {
			resource = seizable->getResource();
		} else { // assume SET
			SeizableItem::SelectionRule rule = seizable->getSelectionRule();
			Set* set = seizable->getSet();
			unsigned int index = 0;
			switch (rule) { // @TODO: Rules for release are DIFFERENT (least and first member seized)
				case SeizableItem::SelectionRule::CYCLICAL:
					index = (seizable->getLastMemberSeized() + 1) % _releaseRequests->list()->size();
					break;
				case SeizableItem::SelectionRule::LARGESTREMAININGCAPACITY:
					// @TODO
					break;
				case SeizableItem::SelectionRule::RANDOM:
					index = trunc(rand() * _releaseRequests->list()->size());
					break;
				case SeizableItem::SelectionRule::SMALLESTNUMBERBUSY:
					// @TODO
					break;
				case SeizableItem::SelectionRule::SPECIFICMEMBER:
					index = _parentModel->parseExpression(seizable->getIndex());
					break;
			}
			_parentModel->getTracer()->traceSimulation(this, "Member of set " + set->getName() + " chosen index " + std::to_string(index), TraceManager::Level::L8_detailed);
			resource = static_cast<Resource*> (set->getElementSet()->getAtRank(index));
			assert(resource != nullptr);
		}
		unsigned int quantity = _parentModel->parseExpression(seizable->getQuantityExpression());
		assert(resource->getNumberBusy() >= quantity); // 202104 ops. maybe not anymore
		_parentModel->getTracer()->traceSimulation(this, _parentModel->getSimulation()->getSimulatedTime(), entity, this, entity->getName() + " releases " + std::to_string(quantity) + " units of resource \"" + resource->getName() + "\" seized on time " + std::to_string(resource->getLastTimeSeized()));
		resource->release(quantity); //{releases and sets the 'LastTimeSeized'property}
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
		unsigned short numRequests = fields->loadField("resquests", DEFAULT.releaseRequestSize);
		for (unsigned short i = 0; i < numRequests; i++) {
			SeizableItem* item = new SeizableItem(nullptr);
			item->setElementManager(_parentModel->getDataManager());
			item->loadInstance(fields, i);
			this->_releaseRequests->insert(item);
		}
	}
	return res;
}

void Release::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("priority", _priority, DEFAULT.priority, saveDefaultValues);
	fields->saveField("resquests", _releaseRequests->size(), DEFAULT.releaseRequestSize, saveDefaultValues);
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
			_attachedDataInsert("SeizableItem" + strIndex(i), resource);
		} else if (seizable->getSeizableType() == SeizableItem::SeizableType::SET) {
			Set* set = seizable->getSet();
			if (set == nullptr && _parentModel->isAutomaticallyCreatesModelDataDefinitions()) {
				set = _parentModel->getParentSimulator()->getPlugins()->newInstance<Set>(_parentModel);
			}
			_attachedDataInsert("SeizableItem" + strIndex(i), set);
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
