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
#include "Model.h"
#include "Resource.h"
#include "Attribute.h"
#include <assert.h>
#include <cmath>

Release::Release(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<Release>(), name) {
}

std::string Release::show() {
	std::string txt = ModelComponent::show() +
			"priority=" + std::to_string(_priority) +
			"releaseRequests={";
	unsigned short i = 0;
	for (std::list<SeizableItemRequest*>::iterator it = _releaseRequests->list()->begin(); it != _releaseRequests->list()->end(); it++, i++) {
		txt += "request" + std::to_string(i) + "=[" + _releaseRequests->show() + "],";
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

List<SeizableItemRequest*>* Release::getReleaseRequests() const {
	return _releaseRequests;
}

//void Release::setResource(Resource* _resource) {
//	this->_resource = _resource;
//}

//Resource* Release::resource() const {
//	return _resource;
//}

void Release::_execute(Entity* entity) {
	for (SeizableItemRequest* seizable : *_releaseRequests->list()) {
		Resource* resource;
		if (seizable->getSeizableType() == SeizableItemRequest::SeizableType::RESOURCE) {
			resource = seizable->getResource();
		} else { // assume SET
			SeizableItemRequest::SelectionRule rule = seizable->getSelectionRule();
			Set* set = seizable->getSet();
			unsigned int index = 0;
			switch (rule) { // \todo: Rules for release are DIFFERENT (least and first member seized)
				case SeizableItemRequest::SelectionRule::CYCLICAL:
					index = (seizable->getLastMemberSeized() + 1) % _releaseRequests->list()->size();
					break;
				case SeizableItemRequest::SelectionRule::LARGESTREMAININGCAPACITY:
					// \todo
					break;
				case SeizableItemRequest::SelectionRule::RANDOM:
					index = trunc(rand() * _releaseRequests->list()->size());
					break;
				case SeizableItemRequest::SelectionRule::SMALLESTNUMBERBUSY:
					// \todo
					break;
				case SeizableItemRequest::SelectionRule::SPECIFICMEMBER:
					index = _parentModel->parseExpression(seizable->getIndex());
					break;
			}
			_parentModel->getTracer()->trace("Member of set " + set->getName() + " chosen index " + std::to_string(index), Util::TraceLevel::L7_detailed);
			resource = static_cast<Resource*> (set->getElementSet()->getAtRank(index));
			assert(resource != nullptr);
		}
		unsigned int quantity = _parentModel->parseExpression(seizable->getQuantityExpression());
		assert(resource->getNumberBusy() >= quantity); // 202104 ops. maybe not anymore
		_parentModel->getTracer()->traceSimulation(_parentModel->getSimulation()->getSimulatedTime(), entity, this, "Entity frees " + std::to_string(quantity) + " units of resource \"" + resource->getName() + "\" seized on time " + std::to_string(resource->getLastTimeSeized()));
		resource->release(quantity, _parentModel->getSimulation()->getSimulatedTime()); //{releases and sets the 'LastTimeSeized'property}
	}
	_parentModel->sendEntityToComponent(entity, this->getNextComponents()->getFrontConnection(), 0.0);
}

void Release::_initBetweenReplications() {
	//for (SeizableItemRequest* seizable : *_releaseRequests->list()) {
	//	if (seizable->getSeizableType() == SeizableItemRequest::SeizableType::RESOURCE)
	//		seizable->getResource()->initBetweenReplications();
	//	else if (seizable->getSeizableType() == SeizableItemRequest::SeizableType::SET)
	//		seizable->getSet()->initBetweenReplications();
	//}
}

bool Release::_loadInstance(std::map<std::string, std::string>* fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		this->_priority = std::stoi(LoadField(fields, "priority", "0"));
		unsigned short numRequests = LoadField(fields, "resquestSize", DEFAULT.releaseRequestSize);
		for (unsigned short i = 0; i < numRequests; i++) {
			SeizableItemRequest* itemRequest = new SeizableItemRequest(nullptr);
			itemRequest->setComponentManager(_parentModel->getComponents());
			itemRequest->loadInstance(fields, i);
			//Resource* resource = static_cast<Resource*> (_parentModel->getElements()->getElement(Util::TypeOf<Resource>(), itemRequest->getResourceName()));
			//itemRequest->setResource(resource);
			//this->_releaseRequests->insert(itemRequest);
		}
	}
	return res;
}

std::map<std::string, std::string>* Release::_saveInstance() {
	std::map<std::string, std::string>* fields = ModelComponent::_saveInstance(); //Util::TypeOf<Release>());
	if (_priority != 0) SaveField(fields, "priority", std::to_string(this->_priority));
	SaveField(fields, "resquestSize", _releaseRequests->size(), DEFAULT.releaseRequestSize);
	unsigned short i = 0;
	for (SeizableItemRequest* request : *_releaseRequests->list()) {
		std::map<std::string, std::string>* seizablefields = request->saveInstance(i);
		fields->insert(seizablefields->begin(), seizablefields->end());
		i++;
	}
	return fields;
}

bool Release::_check(std::string* errorMessage) {
	bool resultAll = true;

	for (SeizableItemRequest* seizable : * _releaseRequests->list()) {
		resultAll &= _parentModel->checkExpression(seizable->getQuantityExpression(), "quantity", errorMessage);
		if (seizable->getSeizableType() == SeizableItemRequest::SeizableType::RESOURCE) {
			resultAll &= _parentModel->getElements()->check(Util::TypeOf<Resource>(), seizable->getResource(), "Resource", errorMessage);
		} else if (seizable->getSeizableType() == SeizableItemRequest::SeizableType::SET) {
			resultAll &= _parentModel->getElements()->check(Util::TypeOf<Set>(), seizable->getSet(), "Set", errorMessage);
		}
		// \todo: Should be checking saveAttribute, index, etc
	}
	return resultAll;
}

//void Release::setResourceName(std::string resourceName) throw () {
//	ModelElement* resource = _parentModel->elements()->element(Util::TypeOf<Resource>(), resourceName);
//	if (resource != nullptr) {
//		this->_resource = dynamic_cast<Resource*> (resource);
//	} else {
//		throw std::invalid_argument("Resource does not exist");
//	}
//}

//std::string Release::resourceName() const {
//	return _resource->name();
//}

PluginInformation* Release::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Release>(), &Release::LoadInstance);
	info->insertDynamicLibFileDependence("resource.so");
	return info;
}

ModelComponent* Release::LoadInstance(Model* model, std::map<std::string, std::string>* fields) {
	Release* newComponent = new Release(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newComponent;

}
