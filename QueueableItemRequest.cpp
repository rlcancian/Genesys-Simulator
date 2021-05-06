/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   QueueableItemRequest.cpp
 * Author: rlcancian
 * 
 * Created on 23 de abril de 2021, 15:09
 */

#include "QueueableItemRequest.h"
#include "ModelElement.h"
#include "Model.h"

QueueableItemRequest::QueueableItemRequest(ModelElement* queueOrSet, QueueableItemRequest::QueueableType queueableType, std::string index) {
	_queueableType = queueableType;
	_queueOrSet = queueOrSet;
	_index = index;
}

/* Queue* QueueableItemRequest::getRequestedQueue() const {
	if (_queueableType == QueueableItemRequest::QueueableType::QUEUE) {
		return static_cast<Queue*> (_queueOrSet);
	} else {
		// assume it is a set ==> _queueableType == QueueableItemRequest::QueueableType::SET
		unsigned int index = this-> ??? ->parse(...)
	}
} */
bool QueueableItemRequest::loadInstance(std::map<std::string, std::string>* fields) {
	bool res = true;
	try {
		_queueableType = static_cast<QueueableItemRequest::QueueableType> (LoadField(fields, "queueableType", static_cast<int> (DEFAULT.queueableType)));
		_queueableName = LoadField(fields, "queueable", "");
		_index = LoadField(fields, "index", DEFAULT.index);
		if (_componentManager != nullptr) {
			this->_queueOrSet = _componentManager->find(_queueableName);
		}
	} catch (...) {
		res = false;
	}
	return res;
}

std::map<std::string, std::string>* QueueableItemRequest::saveInstance() {
	std::map<std::string, std::string>* fields = new std::map<std::string, std::string>();
	SaveField(fields, "queueableType", static_cast<int> (_queueableType), static_cast<int> (DEFAULT.queueableType));
	SaveField(fields, "queueId", _queueOrSet->getId());
	SaveField(fields, "queueable", _queueOrSet->getName(), "");
	SaveField(fields, "index", _index, DEFAULT.index);
	return fields;
}

/*
bool QueueableItemRequest::_loadInstance(std::map<std::string, std::string>* fields, unsigned int parentIndex) {
	bool res = true;
	std::string num = std::to_string(parentIndex);
	try {
		_queueableType = static_cast<QueueableItemRequest::QueueableType> (LoadField(fields, "queueableType" + num, static_cast<int> (DEFAULT.queueableType)));
		_queueableName = LoadField(fields, "queueable" + num, "");
		_index = LoadField(fields, "index" + num, DEFAULT.index);
	} catch (...) {
		res = false;
	}
	return res;
}

std::map<std::string, std::string>* QueueableItemRequest::_saveInstance(unsigned int parentIndex) {
	std::map<std::string, std::string>* fields = new std::map<std::string, std::string>();
	std::string num = std::to_string(parentIndex);
	SaveField(fields, "queueableType" + num, static_cast<int> (_queueableType), static_cast<int> (DEFAULT.queueableType));
	//SaveField(fields, "queueId" + num, _queueOrSet->getId());
	SaveField(fields, "queueable" + num, _queueOrSet->getName(), "");
	SaveField(fields, "index" + num, _index, DEFAULT.index);
	return fields;
}
 */


std::string QueueableItemRequest::show() {
	return "queueType=" + std::to_string(static_cast<int> (_queueableType)) + ",queue=\"" + _queueOrSet->getName() + "\",index=\"" + _index + "\"";
}

void QueueableItemRequest::setIndex(std::string index) {
	this->_index = _index;
}

std::string QueueableItemRequest::getIndex() const {
	return _index;
}

std::string QueueableItemRequest::getQueueableName() const {
	return _queueableName;
}

void QueueableItemRequest::setQueue(Queue* queue) {
	this->_queueOrSet = queue;
	_queueableName = queue->getName();
}

Queue* QueueableItemRequest::getQueue() const {
	return static_cast<Queue*> (_queueOrSet);
}

void QueueableItemRequest::setSet(Set* set) {
	this->_queueOrSet = set;
	_queueableName = set->getName();
}

Set* QueueableItemRequest::getSet() const {
	return static_cast<Set*> (_queueOrSet);
}

void QueueableItemRequest::setQueueableType(QueueableItemRequest::QueueableType queueType) {
	this->_queueableType = _queueableType;
}

QueueableItemRequest::QueueableType QueueableItemRequest::getQueueableType() const {
	return _queueableType;
}

ModelElement* QueueableItemRequest::getQueueable() const {
	return _queueOrSet;
}

void QueueableItemRequest::setComponentManager(ComponentManager* _componentManager) {
	this->_componentManager = _componentManager;
}
