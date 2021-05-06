/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Seize.cpp
 * Author: rafael.luiz.cancian
 * 
 * Created on 21 de Agosto de 2018, 16:17
 */

#include "Seize.h"
#include "Resource.h"
#include "Attribute.h"
#include <assert.h>
#include <cmath>

Seize::Seize(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<Seize>(), name) {
}

std::string Seize::show() {
	std::string txt = ModelComponent::show() +
			"priority=" + std::to_string(_priority) +
			"seizeRequests={";
	unsigned short i = 0;
	for (std::list<SeizableItemRequest*>::iterator it = _seizeRequests->list()->begin(); it != _seizeRequests->list()->end(); it++, i++) {
		txt += "request" + std::to_string(i) + "=[" + _seizeRequests->show() + "],";
	}
	txt = txt.substr(0, txt.length() - 1) + "}";
	return txt;
}

void Seize::setPriority(unsigned short _priority) {
	this->_priority = _priority;
}

unsigned short Seize::getPriority() const {
	return _priority;
}

void Seize::setAllocationType(unsigned int _allocationType) {
	this->_allocationType = _allocationType;
}

unsigned int Seize::getAllocationType() const {
	return _allocationType;
}

/*
void Seize::setQueueName(std::string queueName) throw () {
	Queue* queue = dynamic_cast<Queue*> (_parentModel->getElements()->getElement(Util::TypeOf<Queue>(), queueName));
	if (queue != nullptr) {
		_queue = queue;
	} else {
		throw std::invalid_argument("Queue does not exist");
	}
}

//void Seize::setResourceName(std::string resourceName) throw () {
//	Resource* resource = dynamic_cast<Resource*> (_parentModel->elements()->element(Util::TypeOf<Resource>(), resourceName));
//	if (resource != nullptr) {
//		_resource = resource;
//	} else {
//		throw std::invalid_argument("Resource does not exist");
//	}
//}

//std::string Seize::getResourceName() const {
//	return _seizeRequest->resource()->name();
//}

std::string Seize::getQueueName() const {
	return _queue->getName();
}

//void Seize::setResource(Resource* resource) {
//	this->_resource = resource;
//	_resource->addReleaseResourceEventHandler(Resource::SetResourceEventHandler<Seize>(&Seize:://_handlerForResourceEvent, this));
//}

//Resource* Seize::getResource() const {
//	return _resource;
//}

void Seize::setQueue(Queue* queue) {
	this->_queue = queue;
}

Queue* Seize::getQueue() const {
	return _queue;
}
 */

void Seize::setQueue(Queue* queue) {
	setQueueableItem(new QueueableItemRequest(queue));
}

Queue* Seize::_getQueue() const {
	if (_queueableItem->getQueueableType() == QueueableItemRequest::QueueableType::QUEUE) {
		return _queueableItem->getQueue();
	} else {
		//assume it is a SET
		unsigned int index = _parentModel->parseExpression(_queueableItem->getIndex());
		return static_cast<Queue*> (_queueableItem->getSet()->getElementSet()->getAtRank(index));
	}
}

void Seize::_handlerForResourceEvent(Resource* resource) {
	Queue* queue = _getQueue();
	Waiting* first = queue->first();
	if (first != nullptr) { // there are entities waiting in the queue
		// find quantity requested for such resource
		for (SeizableItemRequest* request : *_seizeRequests->list()) {
			if (request->getResource() == resource) {
				unsigned int quantity = _parentModel->parseExpression(request->getQuantityExpression());
				if ((resource->getCapacity() - resource->getNumberBusy()) >= quantity) { //enought quantity to seize
					double tnow = _parentModel->getSimulation()->getSimulatedTime();
					resource->seize(quantity, tnow);
					_parentModel->getFutureEvents()->insert(new Event(tnow, first->getEntity(), this->getNextComponents()->getFrontConnection()));
					queue->removeElement(first);
					_parentModel->getTracer()->traceSimulation(tnow, first->getEntity(), this, "Waiting entity " + std::to_string(first->getEntity()->entityNumber()) + " now seizes " + std::to_string(quantity) + " elements of resource \"" + resource->getName() + "\"");
				}
			}
		}
	}
}

List<SeizableItemRequest*>* Seize::getSeizeRequests() const {
	return _seizeRequests;
}

void Seize::setQueueableItem(QueueableItemRequest* _queueableItem) {
	this->_queueableItem = _queueableItem;
}

QueueableItemRequest* Seize::getQueueableItem() const {
	return _queueableItem;
}

void Seize::_execute(Entity* entity) {
	for (SeizableItemRequest* seizable : *_seizeRequests->list()) {
		Resource* resource;
		if (seizable->getSeizableType() == SeizableItemRequest::SeizableType::RESOURCE) {
			resource = seizable->getResource();
		} else { // assume SET
			SeizableItemRequest::SelectionRule rule = seizable->getSelectionRule();
			Set* set = seizable->getSet();
			unsigned int index = 0;
			switch (rule) {
				case SeizableItemRequest::SelectionRule::CYCLICAL:
					index = (seizable->getLastMemberSeized() + 1) % _seizeRequests->list()->size();
					break;
				case SeizableItemRequest::SelectionRule::LARGESTREMAININGCAPACITY:
					// \todo
					break;
				case SeizableItemRequest::SelectionRule::RANDOM:
					index = std::trunc(rand() * _seizeRequests->list()->size());
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
		if (resource->getCapacity() - resource->getNumberBusy() < quantity) { // not enought free quantity to allocate. Entity goes to the queue
			WaitingResource* waitingRec = new WaitingResource(entity, this, _parentModel->getSimulation()->getSimulatedTime(), quantity);
			Queue* queue;
			if (_queueableItem->getQueueableType() == QueueableItemRequest::QueueableType::QUEUE) {
				queue = _queueableItem->getQueue();
			} else { // assume SET
				Set* set = _queueableItem->getSet();
				unsigned int index = _parentModel->parseExpression(_queueableItem->getIndex());
				_parentModel->getTracer()->trace("Member of set " + set->getName() + " chosen index " + std::to_string(index), Util::TraceLevel::L7_detailed);
				queue = static_cast<Queue*> (set->getElementSet()->getAtRank(index));
			}
			queue->insertElement(waitingRec); // ->list()->insert(waitingRec);
			_parentModel->getTracer()->traceSimulation(_parentModel->getSimulation()->getSimulatedTime(), entity, this, "Entity starts to wait for resource in queue \"" + queue->getName() + "\" with " + std::to_string(queue->size()) + " elements");
			return;
		} else { // alocate the resource
			resource->seize(quantity, _parentModel->getSimulation()->getSimulatedTime());
			_parentModel->getTracer()->traceSimulation(_parentModel->getSimulation()->getSimulatedTime(), entity, this, "Entity seizes " + std::to_string(quantity) + " elements of resource \"" + resource->getName() + "\" (capacity:" + std::to_string(resource->getCapacity()) + ", numberbusy:" + std::to_string(resource->getNumberBusy()) + ")");
		}
	}
	_parentModel->sendEntityToComponent(entity, this->getNextComponents()->getFrontConnection(), 0.0);
}

void Seize::_initBetweenReplications() {
	//ModelElement::InitBetweenReplications(_queueableItem->getQueueable());
	//for (std::list<SeizableItemRequest*>::iterator it = _seizeRequests->list()->begin(); it != _seizeRequests->list()->end(); it++) {
	//	(*it)->setLastMemberSeized(0);
	//	ModelElement::InitBetweenReplications((*it)->getSeizable());
	//}
}

bool Seize::_loadInstance(std::map<std::string, std::string>* fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		this->_allocationType = LoadField(fields, "allocationType", DEFAULT.allocationType);
		this->_priority = LoadField(fields, "priority", DEFAULT.priority);
		//std::string queueName = LoadField(fields, "queue", "");
		//Queue* queue = dynamic_cast<Queue*> (_parentModel->getElements()->getElement(Util::TypeOf<Queue>(), queueName));
		//this->_queue = queue;
		//QueueableItem
		QueueableItemRequest* queueable = new QueueableItemRequest(nullptr);
		queueable->setComponentManager(_parentModel->getComponents());
		queueable->loadInstance(fields);
		// \TODO: It may be a QUEUE or a SET. Shoud create ANY of them in the model! (QueueableItemRequest can't do it since it's not a ModelElement)
		// SeizableItemRequests
		unsigned short numRequests = LoadField(fields, "resquestSize", DEFAULT.seizeRequestSize);
		for (unsigned short i = 0; i < numRequests; i++) {
			SeizableItemRequest* itemRequest = new SeizableItemRequest(nullptr);
			itemRequest->setComponentManager(_parentModel->getComponents());
			itemRequest->loadInstance(fields, i);
			// \TODO: It may be a RESOURCE or a SET. Shoud create ANY of them in the model! (SeizableItemRequest can't do it since it's not a ModelElement)
			//Resource* resource = static_cast<Resource*> (_parentModel->getElements()->getElement(Util::TypeOf<Resource>(), itemRequest->getResourceName()));
			//itemRequest->setResource(resource);
			//this->_seizeRequests->insert(itemRequest);
		}

	}
	return res;
}

std::map<std::string, std::string>* Seize::_saveInstance() {
	std::map<std::string, std::string>* fields = ModelComponent::_saveInstance(); //Util::TypeOf<Seize>());
	SaveField(fields, "allocationType", _allocationType, DEFAULT.allocationType);
	SaveField(fields, "priority=", _priority, DEFAULT.priority);
	//SaveField(fields, "queueId", _queue->getId());
	//SaveField(fields, "queue", _queue->getName());
	std::map<std::string, std::string>* queueablefields = _queueableItem->saveInstance();
	fields->insert(queueablefields->begin(), queueablefields->end());
	SaveField(fields, "resquestSize", _seizeRequests->size(), DEFAULT.seizeRequestSize);
	unsigned short i = 0;
	for (SeizableItemRequest* request : *_seizeRequests->list()) {
		std::map<std::string, std::string>* seizablefields = request->saveInstance(i);
		fields->insert(seizablefields->begin(), seizablefields->end());
		i++;
	}
	return fields;
}

bool Seize::_check(std::string* errorMessage) {
	bool resultAll = true;
	for (SeizableItemRequest* seizable : *_seizeRequests->list()) {
		resultAll &= _parentModel->checkExpression(seizable->getQuantityExpression(), "quantity", errorMessage);
		if (seizable->getSeizableType() == SeizableItemRequest::SeizableType::RESOURCE) {
			resultAll &= _parentModel->getElements()->check(Util::TypeOf<Resource>(), seizable->getResource(), "Resource", errorMessage);
		} else if (seizable->getSeizableType() == SeizableItemRequest::SeizableType::SET) {
			resultAll &= _parentModel->getElements()->check(Util::TypeOf<Set>(), seizable->getSet(), "Set", errorMessage);
		}
		// \todo: Should be checking saveAttribute, index, etc
	}
	// \todo Check QueueableItem
	if (_queueableItem->getQueueableType() == QueueableItemRequest::QueueableType::QUEUE) {
		resultAll &= _parentModel->getElements()->check(Util::TypeOf<Queue>(), _queueableItem->getQueue(), "Queue", errorMessage);
	} else if (_queueableItem->getQueueableType() == QueueableItemRequest::QueueableType::SET) {
		resultAll &= _parentModel->getElements()->check(Util::TypeOf<Set>(), _queueableItem->getSet(), "Set", errorMessage);
	}
	//resultAll &= _parentModel->getElements()->check(Util::TypeOf<Attribute>(), _saveAttribute, "SaveAttribute", false, errorMessage);
	return resultAll;
}

PluginInformation* Seize::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Seize>(), &Seize::LoadInstance);
	info->insertDynamicLibFileDependence("queue.so");
	info->insertDynamicLibFileDependence("resource.so");
	return info;
}

ModelComponent* Seize::LoadInstance(Model* model, std::map<std::string, std::string>* fields) {
	Seize* newComponent = new Seize(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newComponent;

}

