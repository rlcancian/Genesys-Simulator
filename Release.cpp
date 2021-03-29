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

Release::Release(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<Release>(), name) {
}

std::string Release::show() {
	return ModelComponent::show() +
			",resourceType=" + std::to_string(static_cast<int> (this->_resourceType)) +
			",resource=\"" + this->_releaseRequest->resource()->getName() + "\"" +
			",quantity=" + this->_releaseRequest->quantityExpression();
}

void Release::setPriority(unsigned short _priority) {
	this->_priority = _priority;
}

unsigned short Release::priority() const {
	return _priority;
}

void Release::setResourceType(Resource::ResourceType _resourceType) {
	this->_resourceType = _resourceType;
}

Resource::ResourceType Release::resourceType() const {
	return _resourceType;
}

//void Release::setQuantity(std::string _quantity) {
//	this->_quantityExpression = _quantity;
//}

//std::string Release::quantity() const {
//	return _quantityExpression;
//}

void Release::setRule(Resource::ResourceRule _rule) {
	this->_rule = _rule;
}

Resource::ResourceRule Release::rule() const {
	return _rule;
}

void Release::setSaveAttribute(std::string _saveAttribute) {
	this->_saveAttribute = _saveAttribute;
}

std::string Release::saveAttribute() const {
	return _saveAttribute;
}

void Release::setReleaseRequest(ResourceItemRequest* _releaseRequest) {
	this->_releaseRequest = _releaseRequest;
}

ResourceItemRequest* Release::releaseRequest() const {
	return _releaseRequest;
}

//void Release::setResource(Resource* _resource) {
//	this->_resource = _resource;
//}

//Resource* Release::resource() const {
//	return _resource;
//}

void Release::_execute(Entity* entity) {
	Resource* resource = nullptr;
	if (this->_resourceType == Resource::ResourceType::SET) {
		/*  \todo: +: not implemented yet */
	} else {
		resource = this->_releaseRequest->resource();
	}
	unsigned int quantity = _parentModel->parseExpression(this->_releaseRequest->quantityExpression());
	assert(_releaseRequest->resource()->getNumberBusy() >= quantity);
	_parentModel->getTracer()->traceSimulation(_parentModel->getSimulation()->getSimulatedTime(), entity, this, "Entity frees " + std::to_string(quantity) + " units of resource \"" + resource->getName() + "\" seized on time " + std::to_string(_releaseRequest->resource()->getLastTimeSeized()));
	_releaseRequest->resource()->release(quantity, _parentModel->getSimulation()->getSimulatedTime()); //{releases and sets the 'LastTimeSeized'property}
	_parentModel->sendEntityToComponent(entity, this->getNextComponents()->getFrontConnection(), 0.0);
}

void Release::_initBetweenReplications() {
	this->_releaseRequest->resource()->initBetweenReplications();
}

bool Release::_loadInstance(std::map<std::string, std::string>* fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		this->_priority = std::stoi((*(fields->find("priority"))).second);
		this->_resourceType = static_cast<Resource::ResourceType> (std::stoi((*(fields->find("resourceType"))).second));
		this->_rule = static_cast<Resource::ResourceRule> (std::stoi((*(fields->find("rule"))).second));
		this->_saveAttribute = ((*(fields->find("saveAttribute"))).second);
		//Util::identitifcation resourceId = std::stoi((*(fields->find("resourceId"))).second);
		//Resource* res = dynamic_cast<Resource*> (_model->elements()->element(Util::TypeOf<Resource>(), resourceId));
		std::string resourceName = ((*(fields->find("resourceName"))).second);
		Resource* res = dynamic_cast<Resource*> (_parentModel->getElements()->getElement(Util::TypeOf<Resource>(), resourceName));
		this->_releaseRequest = new ResourceItemRequest(res, (*(fields->find("quantity"))).second);
		//this->_releaseRequest->setQuantityExpression(());
		//this->_releaseRequest->setResource(res);
	}
	return res;
}

std::map<std::string, std::string>* Release::_saveInstance() {
	std::map<std::string, std::string>* fields = ModelComponent::_saveInstance(); //Util::TypeOf<Release>());
	fields->emplace("priority", std::to_string(this->_priority));
	fields->emplace("quantity", "\"" + this->_releaseRequest->quantityExpression() + "\"");
	fields->emplace("resourceType", std::to_string(static_cast<int> (this->_resourceType)));
	fields->emplace("resourceId", std::to_string(this->_releaseRequest->resource()->getId()));
	fields->emplace("resourceName", (this->_releaseRequest->resource()->getName()));
	fields->emplace("rule", std::to_string(static_cast<int> (this->_rule)));
	fields->emplace("saveAttribute", this->_saveAttribute);
	return fields;

}

bool Release::_check(std::string* errorMessage) {
	bool resultAll = true;
	resultAll &= _parentModel->checkExpression(_releaseRequest->quantityExpression(), "quantity", errorMessage);
	resultAll &= _parentModel->getElements()->check(Util::TypeOf<Resource>(), _releaseRequest->resource(), "resource", errorMessage);
	resultAll &= _parentModel->getElements()->check(Util::TypeOf<Attribute>(), _saveAttribute, "SaveAttribute", false, errorMessage);
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
