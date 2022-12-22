/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Batch.cpp
 * Author: rlcancian
 * 
 * Created on 03 de Junho de 2019, 15:14
 */

#include "Batch.h"
#include <cassert>
#include "../../kernel/simulator/Model.h"
#include "../../kernel/simulator/Attribute.h"
#include "../../plugins/data/EntityGroup.h"
#include "../../kernel/simulator/Simulator.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &Batch::GetPluginInformation;
}
#endif

ModelDataDefinition* Batch::NewInstance(Model* model, std::string name) {
	return new Batch(model, name);
}

Batch::Batch(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<Batch>(), name) {
}

std::string Batch::show() {
	return ModelComponent::show() + "";
}

ModelComponent* Batch::LoadInstance(Model* model, PersistenceRecord *fields) {
	Batch* newComponent = new Batch(model);
	try {
		newComponent->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newComponent;
}

void Batch::setGroupedEntityType(EntityType* groupedEntityType) {
	this->_groupedEntityType = groupedEntityType;
}

void Batch::setGroupedEntityTypeName(std::string groupedEntityTypeName) {
	ModelDataDefinition* data = _parentModel->getDataManager()->getDataDefinition(Util::TypeOf<EntityType>(), groupedEntityTypeName);
	if (data != nullptr) {
		this->_groupedEntityType = dynamic_cast<EntityType*> (data);
	} else {
		this->_groupedEntityType = new EntityType(_parentModel, groupedEntityTypeName);
	}
}

EntityType* Batch::getGroupedEntityType() const {
	return _groupedEntityType;
}

void Batch::setAttributeName(std::string attributeName) {
	this->_attributeName = attributeName;
}

std::string Batch::getAttributeName() const {
	return _attributeName;
}

void Batch::setBatchSize(std::string batchSize) {
	this->_batchSize = batchSize;
}

std::string Batch::getBatchSize() const {
	return _batchSize;
}

void Batch::setRule(Batch::Rule _rule) {
	this->_rule = _rule;
}

Batch::Rule Batch::getRule() const {
	return _rule;
}

void Batch::setGroupedAttributes(Batch::GroupedAttribs _groupedAttributes) {
	this->_groupedAttributes = _groupedAttributes;
}

Batch::GroupedAttribs Batch::getGroupedAttributes() const {
	return _groupedAttributes;
}

void Batch::_onDispatchEvent(Entity* entity, unsigned int inputPortNumber) {
	double tnow = _parentModel->getSimulation()->getSimulatedTime();
	_queue->insertElement(new Waiting(entity, tnow, this, 0));
	unsigned int batchSize = _parentModel->parseExpression(_batchSize);
	std::list<Waiting*>* entitiesToGroup = nullptr;
	// check if batch size is complete
	if (_rule == Batch::Rule::Any) {
		if (_queue->size() == batchSize) {
			entitiesToGroup = new std::list<Waiting*>();
			for (unsigned int i = 0; i < _queue->size(); i++) {
				entitiesToGroup->insert(entitiesToGroup->end(), _queue->getAtRank(i));
			}
			_parentModel->getTracer()->traceSimulation(this, TraceManager::Level::L7_internal, "Queue has " + std::to_string(_queue->size()) + " elements and a group with " + std::to_string(batchSize) + " elements will be created");

		} else {
			_parentModel->getTracer()->traceSimulation(this, "Queue has " + std::to_string(_queue->size()) + " elements, not enought to form a group with " + std::to_string(batchSize));
		}
	} else if (_rule == Batch::Rule::ByAttribute) {// rule IS Batch::Rule::ByAttribute
		std::map<double, unsigned int>* countByValue = new std::map<double, unsigned int>();
		double value, testValue;
		Waiting* we;
		std::map<double, unsigned int>::iterator it;
		for (unsigned int i = 0; i < _queue->size(); i++) {
			value = _queue->getAtRank(i)->getEntity()->getAttributeValue(_attributeName);
			while ((it = countByValue->find(value)) == countByValue->end()) {// first time this value shows up
				countByValue->insert({value, 0u});
			}
			(*it).second = (*it).second + 1;
			if ((*it).second == batchSize) {
				entitiesToGroup = new std::list<Waiting*>();
				value = (*it).first;
				for (unsigned int j = 0; j < _queue->size(); j++) {
					we = _queue->getAtRank(j);
					testValue = we->getEntity()->getAttributeValue(_attributeName);
					if (testValue == value) {
						entitiesToGroup->insert(entitiesToGroup->end(), we);
					}
				}
				_parentModel->getTracer()->traceSimulation(this, TraceManager::Level::L7_internal, "Found " + std::to_string(entitiesToGroup->size()) + " elements in queue with the same value (" + std::to_string(value) + ") for attribute \"" + _attributeName + "\", and a group will be created");
				break; //@TODO //breake? //next? 
			}
		}
	} else { // BY EntityType

	}
	if (entitiesToGroup != nullptr) { // there are enought entities to group in
		//_parentModel->getTracer()->traceSimulation(this, "Queue has " + std::to_string(_queue->size()) + " elements, not enought to form a group");
		assert(entitiesToGroup->size() == batchSize);
		// creates a new entity that represents the group
		Entity* representativeEnt;
		if (_groupedEntityType != nullptr) {
			representativeEnt = _parentModel->createEntity(_groupedEntityType->getName() + "_Group", true);
			representativeEnt->setEntityType(_groupedEntityType);
		} else {
			representativeEnt = _parentModel->createEntity(entity->getEntityType()->getName() + "_Group", true);
			representativeEnt->setEntityType(entity->getEntityType());
		}
		unsigned int groupIdKey = representativeEnt->getId(); // an "EntityGroup" is a MAP, with one LIST for every RepresentativeEntity ID as KEY
		if (_batchType == Batch::BatchType::Temporary) {
			representativeEnt->setAttributeValue("Entity.Group", _entityGroup->getId()); // The "Entity.Group" attribute is the EntityGroup Id (an internel modeldatum of Batch), while the ID of the representative entity is the KEY of the map of that EntityGroup
		}
		// remove all entities from the queue while storing attributes depending on representative
		Entity* enqueuedEnt;
		std::string txtEntsInGroup = "";
		unsigned int i = 0;
		bool accumAttribs;
		for (Waiting* waiting : *entitiesToGroup) {
			enqueuedEnt = waiting->getEntity();
			txtEntsInGroup += enqueuedEnt->getName() + ", ";
			_queue->removeElement(waiting);
			accumAttribs = (i == 0 && _groupedAttributes == Batch::GroupedAttribs::FirstEntity)
					|| (i == (entitiesToGroup->size() - 1) && _groupedAttributes == Batch::GroupedAttribs::LastEntity)
					|| (_groupedAttributes == Batch::GroupedAttribs::SumAttributes);
			if (accumAttribs) {
				std::string attribName;
				double value;
				for (ModelDataDefinition* attrib : *_parentModel->getDataManager()->getDataDefinitionList(Util::TypeOf<Attribute>())->list()) {
					attribName = attrib->getName();
					value = representativeEnt->getAttributeValue(attribName);
					value += enqueuedEnt->getAttributeValue(attribName);
					representativeEnt->setAttributeValue(attribName, value);
				}
			}
			if (_batchType == Batch::BatchType::Temporary) {
				_entityGroup->insertElement(groupIdKey, enqueuedEnt);
			} else {
				_parentModel->removeEntity(enqueuedEnt);
			}
			i++;
		}
		txtEntsInGroup = txtEntsInGroup.substr(0, txtEntsInGroup.length() - 2);
		if (_batchType == Batch::BatchType::Temporary) {
			_parentModel->getTracer()->traceSimulation(this, TraceManager::Level::L7_internal, "Group key " + std::to_string(groupIdKey) + " was created containing entities: " + txtEntsInGroup + " and representative entity wih attribute 'Entity.Group'=" + std::to_string(_entityGroup->getId()));
		} else {
			_parentModel->getTracer()->traceSimulation(this, TraceManager::Level::L7_internal, "Entity \"" + representativeEnt->getName() + "\" id=" + std::to_string(groupIdKey) + " now represented the removed entities: " + txtEntsInGroup);
		}
		this->_parentModel->sendEntityToComponent(representativeEnt, this->getConnections()->getFrontConnection());
	} else {
		if (_rule == Batch::Rule::Any) {
			_parentModel->getTracer()->traceSimulation(this, "Entity \"" + entity->getName() + "\" is waiting in the queue " + _queue->getName());
		} else if (_rule == Batch::Rule::ByAttribute) {
			_parentModel->getTracer()->traceSimulation(this, "Entity \"" + entity->getName() + "\" with attribute '" + _attributeName + "'=" + std::to_string(entity->getAttributeValue(_attributeName)) + " is waiting in the queue " + _queue->getName());
		} else { // by entity type
		}
	}
}

bool Batch::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		_batchType = static_cast<Batch::BatchType> (fields->loadField("BatchType", static_cast<int> (DEFAULT.batchType)));
		_rule = static_cast<Batch::Rule> (fields->loadField("Rule", static_cast<int> (DEFAULT.rule)));
		_groupedAttributes = static_cast<Batch::GroupedAttribs> (fields->loadField("GroupedAttributes", static_cast<int> (DEFAULT.groupedAttributes)));
		_batchSize = fields->loadField("BatchSize", DEFAULT.batchSize);
		_attributeName = fields->loadField("AttributeName", DEFAULT.attributeName);
		std::string groupedEntityTypeName = fields->loadField("GroupedEntityType", "");
		if (groupedEntityTypeName != "") {
			_groupedEntityType = dynamic_cast<EntityType*> (_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<EntityType>(), groupedEntityTypeName));
		}
	}
	return res;
}

void Batch::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("BatchType", static_cast<int> (_batchType), static_cast<int> (DEFAULT.batchType), saveDefaultValues);
	fields->saveField("Rule", static_cast<int> (_rule), static_cast<int> (DEFAULT.rule), saveDefaultValues);
	fields->saveField("GroupedAttributes", static_cast<int> (_groupedAttributes), static_cast<int> (DEFAULT.groupedAttributes), saveDefaultValues);
	fields->saveField("BatchSize", _batchSize, DEFAULT.batchSize, saveDefaultValues);
	fields->saveField("AttributeName", _attributeName, DEFAULT.attributeName, saveDefaultValues);
	if (_groupedEntityType != nullptr) {
		fields->saveField("GroupedEntityType", _groupedEntityType->getName(), "", saveDefaultValues);
	}
}

void Batch::_createInternalAndAttachedData() {
	_attachedAttributesInsert({"Entity.Group"});
	if (_parentModel->isAutomaticallyCreatesModelDataDefinitions()) {
		_attachedDataInsert("GroupdEntityType", _groupedEntityType);
		if (_queue == nullptr) {
			PluginManager* plugins = _parentModel->getParentSimulator()->getPlugins();
			_queue = plugins->newInstance<Queue>(_parentModel, this->getName() + ".Queue");
			_internalDataInsert("EntityQueue", _queue);
			_entityGroup = plugins->newInstance<EntityGroup>(_parentModel, this->getName() + ".EntiyGroup");
			_internalDataInsert("EntityGroup", _entityGroup);
		}
		if (_attributeName != "") {
			ModelDataManager* elements = _parentModel->getDataManager();
			ModelDataDefinition* attribute = elements->getDataDefinition(Util::TypeOf<Attribute>(), _attributeName);
			_attachedDataInsert("AttributeName", attribute);
		}
	}
}

bool Batch::_check(std::string * errorMessage) {
	bool resultAll = true;
	ModelDataManager* elements = _parentModel->getDataManager();
	if (_attributeName != "") {
		resultAll &= elements->getDataDefinition(Util::TypeOf<Attribute>(), _attributeName) == nullptr;
	}
	if (_groupedEntityType != nullptr) {
		resultAll += elements->check(Util::TypeOf<EntityType>(), _groupedEntityType, "Grouped Entity Type", errorMessage);
	}
	if (_attributeName != "") {
		ModelDataDefinition* attribute = elements->getDataDefinition(Util::TypeOf<Attribute>(), _attributeName);
		resultAll &= attribute == nullptr;
	}
	resultAll += _parentModel->checkExpression(_batchSize, "Batch Size", errorMessage);

	return resultAll;
}

PluginInformation * Batch::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Batch>(), &Batch::LoadInstance, &Batch::NewInstance);
	info->insertDynamicLibFileDependence("entitygroup.so");
	info->insertDynamicLibFileDependence("queue.so");
	info->setCategory("Grouping");
	std::string help = "This module is intended as the grouping mechanism within the simulation model.";
	help += " Batches can be permanently or temporarily grouped.";
	help += " Temporary batches must later be split using the Separate module.";
	help += " Batches may be made with any specified number of entering entities or may be matched together based on an attribute.";
	help += " Entities arriving at the Batch module are placed in a queue until the required number of entities has accumulated.";
	help += " Once accumulated, a new representative entity is created.";
	help += " Animation showing the number of entities waiting to be batched, as well as an animated queue for waiting entities, is displayed when the module is placed.";
	help += " TYPICAL USES: (1) Collect a defined number of parts before starting processing; (2) Reassemble previously separated copies of a form;";
	help += " (3) Bring together a patient and his record before commencing an appointment";
	info->setDescriptionHelp(help);
	return info;
}


