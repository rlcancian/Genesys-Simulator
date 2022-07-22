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
#include "../data/Resource.h"
#include "../../kernel/simulator/Attribute.h"
#include "../../kernel/simulator/Simulator.h"
#include <assert.h>
#include <cmath>

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &Seize::GetPluginInformation;
}
#endif

// constructors

ModelDataDefinition* Seize::NewInstance(Model* model, std::string name) {
	return new Seize(model, name);
}

Seize::Seize(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<Seize>(), name) {
	// properties
	_addProperty(new PropertyT<unsigned short>(Util::TypeOf<Seize>(), "Priority",
			DefineGetter<Seize, unsigned short>(this, &Seize::getPriority),
			DefineSetter<Seize, unsigned short>(this, &Seize::setPriority)));
	_addProperty(new PropertyT<std::string>(Util::TypeOf<Seize>(), "Save Attribute",
			DefineGetter<Seize, std::string>(this, &Seize::getSaveAttribute),
			DefineSetter<Seize, std::string>(this, &Seize::setSaveAttribute)));
	//unsigned int _allocationType = DEFAULT.allocationType; // uint ? enum?
	_addProperty(new PropertyT<QueueableItem*>(Util::TypeOf<Seize>(), "Queueable Item",
			DefineGetter<Seize, QueueableItem*>(this, &Seize::getQueueableItem),
			DefineSetter<Seize, QueueableItem*>(this, &Seize::setQueueableItem)));
	//List<SeizableItem*>* _seizeRequests = new List<SeizableItem*>();
	_addProperty(new PropertyT<List<SeizableItem*>*>(Util::TypeOf<Seize>(), "Seize Requests",
			DefineGetter<Seize, List<SeizableItem*>*>(this, &Seize::getSeizeRequests),
			nullptr));
}

// public 

std::string Seize::show() {
	std::string txt = ModelComponent::show() +
			"priority=" + std::to_string(_priority) +
			"seizeRequests={";
	unsigned short i = 0;
	for (SeizableItem* item : *_seizeRequests->list()) {
		txt += "request" + std::to_string(i++) + "=[" + item->show() + "],";
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

void Seize::setQueue(Queue* queue) {
	setQueueableItem(new QueueableItem(queue));
}

List<SeizableItem*>* Seize::getSeizeRequests() const {
	return _seizeRequests;
}

void Seize::setQueueableItem(QueueableItem* _queueableItem) {
	this->_queueableItem = _queueableItem;
}

QueueableItem* Seize::getQueueableItem() const {
	return _queueableItem;
}

void Seize::setSaveAttribute(std::string _saveAttribute) {
	this->_saveAttribute = _saveAttribute;
}

std::string Seize::getSaveAttribute() const {
	return _saveAttribute;
}

// public static

PluginInformation* Seize::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Seize>(), &Seize::LoadInstance, &Seize::NewInstance);
	info->insertDynamicLibFileDependence("queue.so");
	info->insertDynamicLibFileDependence("resource.so");
	std::string help = "The Seize module allocates units of one or more resources to an entity.";
	help += " The Seize module may be used to seize units of a resource, a member of a resource set, or a resource as defined by an alternative method, such as an attribute or expression.";
	help += " When an entity enters this module, it waits in a queue (if specified) until all specified resources are available simultaneously.";
	help += " Allocation type for resource usage is also specified.";
	help += " An animated queue is displayed above the module when the module is placed.";
	help += " TYPICAL USES: (1) Beginning a customer order (seize the operator); (2) Starting a tax return (seize the accountant);";
	help += " (3) Being admitted to hospital (seize the hospital room, nurse, doctor)";
	info->setDescriptionHelp(help);

	return info;
}

ModelComponent* Seize::LoadInstance(Model* model, PersistenceRecord *fields) {
	Seize* newComponent = new Seize(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newComponent;
}

// protected must override

void Seize::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	for (SeizableItem* seizable : *_seizeRequests->list()) {
		Resource* resource = _getResourceFromSeizableItem(seizable, entity);
		unsigned int quantity = _parentModel->parseExpression(seizable->getQuantityExpression());
		if (!resource->seize(quantity)) { // not enought free quantity to allocate. Entity goes to the queue
			WaitingResource* waitingRec = new WaitingResource(entity, _parentModel->getSimulation()->getSimulatedTime(), quantity, this);
			Queue* queue;
			if (_queueableItem->getQueueableType() == QueueableItem::QueueableType::QUEUE) {
				queue = _queueableItem->getQueue();
			} else { // assume SET
				Set* set = _queueableItem->getSet();
				unsigned int index = _parentModel->parseExpression(_queueableItem->getIndex());
				_parentModel->getTracer()->traceSimulation(this, "Member of set " + set->getName() + " chosen index " + std::to_string(index), TraceManager::Level::L8_detailed);
				queue = static_cast<Queue*> (set->getElementSet()->getAtRank(index));
			}
			queue->insertElement(waitingRec); // ->list()->insert(waitingRec);
			_parentModel->getTracer()->traceSimulation(this, _parentModel->getSimulation()->getSimulatedTime(), entity, this, "Entity starts to wait for resource in queue \"" + queue->getName() + "\" with " + std::to_string(queue->size()) + " elements");
			return;
		} else { // alocate the resource
			_parentModel->getTracer()->traceSimulation(this, _parentModel->getSimulation()->getSimulatedTime(), entity, this, entity->getName() + " seizes " + std::to_string(quantity) + " elements of resource \"" + resource->getName() + "\" (capacity:" + std::to_string(resource->getCapacity()) + ", numberbusy:" + std::to_string(resource->getNumberBusy()) + ")");
		}
	}
	_parentModel->sendEntityToComponent(entity, this->getConnections()->getFrontConnection());
}

bool Seize::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		this->_allocationType = fields->loadField("allocationType", DEFAULT.allocationType);
		this->_priority = fields->loadField("priority", DEFAULT.priority);
		this->_saveAttribute = fields->loadField("saveAttribute", DEFAULT.saveAttribute);
		_queueableItem = new QueueableItem(nullptr);
		_queueableItem->setElementManager(_parentModel->getDataManager());
		_queueableItem->loadInstance(fields);
		unsigned short numRequests = fields->loadField("resquests", DEFAULT.seizeRequestSize);
		for (unsigned short i = 0; i < numRequests; i++) {
			SeizableItem* item = new SeizableItem(nullptr);
			item->setElementManager(_parentModel->getDataManager());
			item->loadInstance(fields, i);
			this->_seizeRequests->insert(item);
		}
	}
	return res;
}

void Seize::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("allocationType", _allocationType, DEFAULT.allocationType, saveDefaultValues);
	fields->saveField("priority=", _priority, DEFAULT.priority, saveDefaultValues);
	fields->saveField("saveAttribute=", _saveAttribute, DEFAULT.saveAttribute, saveDefaultValues);
	if (_queueableItem != nullptr) {
		_queueableItem->saveInstance(fields, saveDefaultValues);
	}
	fields->saveField("resquests", _seizeRequests->size(), DEFAULT.seizeRequestSize, saveDefaultValues);
	unsigned short i = 0;
	for (SeizableItem* request : *_seizeRequests->list()) {
		request->saveInstance(fields, i, saveDefaultValues);
		i++;
	}
}

// protected could override

void Seize::_initBetweenReplications() {
	// @TODO CHeck why commented (seizableItems are NOT ModelDataDefinition, therefore they are NOT iniatilized by ModelSimulation
	//ModelDataDefinition::InitBetweenReplications(_queueableItem->getQueueable());
	//for (std::list<SeizableItem*>::iterator it = _seizeRequests->list()->begin(); it != _seizeRequests->list()->end(); it++) {
	//	(*it)->setLastMemberSeized(0);
	//	ModelDataDefinition::InitBetweenReplications((*it)->getSeizable());
	//}
}

void Seize::_createInternalAndAttachedData() {
	int i = 0;
	for (SeizableItem* seizable : *_seizeRequests->list()) {
		if (seizable->getSeizableType() == SeizableItem::SeizableType::RESOURCE) {
			Resource* resource = seizable->getResource();
			if (resource == nullptr && _parentModel->isAutomaticallyCreatesModelDataDefinitions()) {
				resource = _parentModel->getParentSimulator()->getPlugins()->newInstance<Resource>(_parentModel);
				seizable->setResource(resource);
			}
			_attachedDataInsert("SeizableItem" + strIndex(i), resource);
			Resource::ResourceEventHandler handler = Resource::SetResourceEventHandler<Seize>(&Seize::_handlerForResourceEvent, this);
			resource->addReleaseResourceEventHandler(handler, this, _priority);
		} else if (seizable->getSeizableType() == SeizableItem::SeizableType::SET) {
			Set* set = seizable->getSet();
			if (set == nullptr && _parentModel->isAutomaticallyCreatesModelDataDefinitions()) {
				set = _parentModel->getParentSimulator()->getPlugins()->newInstance<Set>(_parentModel);
			}
			_attachedDataInsert("SeizableItem" + strIndex(i), set);
			Resource* rec;
			for (ModelDataDefinition* datum : *seizable->getSet()->getElementSet()->list()) {
				rec = static_cast<Resource*> (datum);
				rec->addReleaseResourceEventHandler(Resource::SetResourceEventHandler<Seize>(&Seize::_handlerForResourceEvent, this), this, _priority);
			}
		}
		i++;
	}
	// Check QueueableItem
	if (_queueableItem == nullptr) {
		//* @TODO: Implement. 
	} else {
		if (_queueableItem->getQueueableType() == QueueableItem::QueueableType::QUEUE) {
			Queue* queue = _queueableItem->getQueue();
			if (queue == nullptr && _parentModel->isAutomaticallyCreatesModelDataDefinitions()) {
				queue = _parentModel->getParentSimulator()->getPlugins()->newInstance<Queue>(_parentModel);
				_queueableItem->setQueue(queue);
			}
			_attachedDataInsert("QueueableItem", queue);
		} else if (_queueableItem->getQueueableType() == QueueableItem::QueueableType::SET) {
			Set* set = _queueableItem->getSet();
			if (set == nullptr && _parentModel->isAutomaticallyCreatesModelDataDefinitions()) {
				set = _parentModel->getParentSimulator()->getPlugins()->newInstance<Set>(_parentModel);
				_queueableItem->setSet(set);
			}
			_attachedDataInsert("QueueableItem", set);
		}
	}

	if (_saveAttribute != "") { // check if saveAttribute is an attribute
		ModelDataDefinition* saveAttr = _parentModel->getDataManager()->getDataDefinition(Util::TypeOf<Attribute>(), _saveAttribute);
		if (saveAttr == nullptr && _parentModel->isAutomaticallyCreatesModelDataDefinitions()) {
			saveAttr = _parentModel->getParentSimulator()->getPlugins()->newInstance<Attribute>(_parentModel);
		}
		_attachedDataInsert("SaveAttribute", saveAttr);
	}
}

bool Seize::_check(std::string* errorMessage) {
	bool resultAll = true;
	// Check SeizableItems
	int i = 0;
	for (SeizableItem* seizable : *_seizeRequests->list()) {
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
	// Check QueueableItem
	if (_queueableItem == nullptr) {
		resultAll = false;
		*errorMessage += "QueueableItem is missing";
	} else {
		if (_queueableItem->getQueueableType() == QueueableItem::QueueableType::QUEUE) {
			// Queue* queue = _queueableItem->getQueue();
			resultAll &= _parentModel->getDataManager()->check(Util::TypeOf<Queue>(), _queueableItem->getQueue(), "Queueable Queue", errorMessage);
		} else if (_queueableItem->getQueueableType() == QueueableItem::QueueableType::SET) {
			// Set* set = _queueableItem->getSet();
			resultAll &= _parentModel->getDataManager()->check(Util::TypeOf<Set>(), _queueableItem->getSet(), "Queueable Set", errorMessage);
		}
	}

	if (_saveAttribute != "") { // check if saveAttribute is an attribute
		// ModelDataDefinition* saveAttr = _parentModel->getDataManager()->getDataDefinition(Util::TypeOf<Attribute>(), _saveAttribute);
		resultAll &= _parentModel->getDataManager()->check(Util::TypeOf<Attribute>(), _saveAttribute, "Save Attribute", true, errorMessage);
	}
	return resultAll;
}


// private

Queue* Seize::_getQueue() const {
	if (_queueableItem->getQueueableType() == QueueableItem::QueueableType::QUEUE) {
		return _queueableItem->getQueue();
	} else {
		//assume it is a SET
		unsigned int index = _parentModel->parseExpression(_queueableItem->getIndex());
		return static_cast<Queue*> (_queueableItem->getSet()->getElementSet()->getAtRank(index));
	}
}

void Seize::_handlerForResourceEvent(Resource* resource) { //@TODO Resource is useless now
	Queue* queue = _getQueue();
	Waiting* first = queue->first();
	if (first != nullptr) { // there are entities waiting in the queue
		// check if all Seize requests can be seized by the entity in the queue
		bool canSeizeAll = true;
		unsigned int quantityRequested, quantityAvailable;
		for (SeizableItem* seizable : *_seizeRequests->list()) {
			Resource* resource = _getResourceFromSeizableItem(seizable, first->getEntity());
			quantityRequested = _parentModel->parseExpression(seizable->getQuantityExpression());
			quantityAvailable = resource->getCapacity() - resource->getNumberBusy();
			if (quantityAvailable < quantityRequested) {
				canSeizeAll = false;
				break;
			}
		}
		if (canSeizeAll) {
			queue->removeElement(first);
			//_parentModel->getTracer()->traceSimulation(this, tnow, first->getEntity(), this, "Waiting entity " + first->getEntity()->getName() + " removed from queue and will try to seize the resources");// now seizes " + std::to_string(quantity) + " elements of resource \"" + resource->getName() + "\"");
			_parentModel->getTracer()->trace("Waiting entity " + first->getEntity()->getName() + " removed from queue and will try to seize the resources"); // now seizes " + std::to_string(quantity) + " elements of resource \"" + resource->getName() + "\"");
			_parentModel->sendEntityToComponent(first->getEntity(), this); // move waiting entity from queue to this component
		}
		/*
		if (request->getResource() == resource) {
			unsigned int quantity = _parentModel->parseExpression(request->getQuantityExpression());
			if ((resource->getCapacity() - resource->getNumberBusy()) >= quantity) { //enought quantity to seize
				double tnow = _parentModel->getSimulation()->getSimulatedTime();
				resource->seize(quantity, tnow);
				_parentModel->getFutureEvents()->insert(new Event(tnow, first->getEntity(), this->getNextComponents()->getFrontConnection()));
				queue->removeElement(first);
				_parentModel->getTracer()->traceSimulation(this, tnow, first->getEntity(), this, "Waiting entity " + first->getEntity()->getName() + " removed from queue and now seizes " + std::to_string(quantity) + " elements of resource \"" + resource->getName() + "\"");
			}
		}
		 */
	}
}

Resource* Seize::_getResourceFromSeizableItem(SeizableItem* seizable, Entity* entity) {
	Resource* resource;
	if (seizable->getSeizableType() == SeizableItem::SeizableType::RESOURCE) {
		resource = seizable->getResource();
	} else { // assume SET
		SeizableItem::SelectionRule rule = seizable->getSelectionRule();
		Set* set = seizable->getSet();
		unsigned int index = 0;
		switch (rule) {
			case SeizableItem::SelectionRule::CYCLICAL:
				index = (seizable->getLastMemberSeized() + 1) % _seizeRequests->list()->size();
				break;
			case SeizableItem::SelectionRule::LARGESTREMAININGCAPACITY:
				// @TODO
				break;
			case SeizableItem::SelectionRule::RANDOM:
				// @TODO: RANDOM IS REALLY A PROBLEM!!! NOW IT MAY CAUSE AN ERROR (DEQUEUE AN ENTITY BECAUSE IT CAN SEIZE ALL REQUESTS, BUT ANOTHER RANDOM REQUEST MY BE SELECTED AFTER, IT IT MAY BE BUSY
				index = std::trunc(rand() * _seizeRequests->list()->size());
				break;
			case SeizableItem::SelectionRule::SMALLESTNUMBERBUSY:
				// @TODO
				break;
			case SeizableItem::SelectionRule::SPECIFICMEMBER:
				index = _parentModel->parseExpression(seizable->getIndex());
				break;
		}
		if (_saveAttribute != "") {
			entity->setAttributeValue(_saveAttribute, index);
		}
		_parentModel->getTracer()->trace("Member of set " + set->getName() + " chosen index " + std::to_string(index), TraceManager::Level::L8_detailed);
		resource = static_cast<Resource*> (set->getElementSet()->getAtRank(index));
		assert(resource != nullptr);
	}
	return resource;
}