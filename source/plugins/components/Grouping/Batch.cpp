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

#include "plugins/components/Grouping/Batch.h"
#include <cassert>
#include "kernel/simulator/Model.h"
#include "kernel/simulator/Attribute.h"
#include "../../data/Grouping/EntityGroup.h"
#include "kernel/simulator/Simulator.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &Batch::GetPluginInformation;
}
#endif

ModelDataDefinition* Batch::NewInstance(Model* model, std::string name) {
	return new Batch(model, name);
}

std::string Batch::convertEnumToStr(BatchType type) {
	switch (static_cast<int> (type)) {
		case 0: return "Temporary";
		case 1: return "Permanent";
	}
	return "Unknown";
}

std::string Batch::convertEnumToStr(Rule rule) {
	switch (static_cast<int> (rule)) {
		case 0: return "Any";
		case 1: return "ByAttribute";
	}
	return "Unknown";
}

std::string Batch::convertEnumToStr(GroupedAttribs attribs) {
	switch (static_cast<int> (attribs)) {
		case 0: return "FirstEntity";
		case 1: return "LastEntity";
		case 2: return "SumAttributes";
	}
	return "Unknown";
}

Batch::Batch(Model* model, std::string name) : ModelComponent(model, Util::TypeOf<Batch>(), name) {
	SimulationControlGenericEnum<Batch::BatchType, Batch>* propBatchType = new SimulationControlGenericEnum<Batch::BatchType, Batch>(
				std::bind(&Batch::getBatchType, this),
				std::bind(&Batch::setBatchType, this, std::placeholders::_1),
				Util::TypeOf<Batch>(), getName(), "BatchType", "");
    SimulationControlGenericEnum<Batch::Rule, Batch>* propRule = new SimulationControlGenericEnum<Batch::Rule, Batch>(
                std::bind(&Batch::getRule, this),
                std::bind(&Batch::setRule, this, std::placeholders::_1),
                Util::TypeOf<Batch>(), getName(), "Rule", "");
    SimulationControlGenericEnum<Batch::GroupedAttribs, Batch>* propGroupedAttribs = new SimulationControlGenericEnum<Batch::GroupedAttribs, Batch>(
                std::bind(&Batch::getGroupedAttributes, this),
                std::bind(&Batch::setGroupedAttributes, this, std::placeholders::_1),
                Util::TypeOf<Batch>(), getName(), "GroupedAttributes", "");
	SimulationControlGenericClass<EntityType*, Model*, EntityType>* propGroupedEntity = new SimulationControlGenericClass<EntityType*, Model*, EntityType>(
				_parentModel,
				std::bind(&Batch::getGroupedEntityType, this),
				std::bind(&Batch::setGroupedEntityType, this, std::placeholders::_1),
				Util::TypeOf<Batch>(), getName(), "GroupedEntityType", "");
	SimulationControlGeneric<std::string>* propAttributeName = new SimulationControlGeneric<std::string>(
				std::bind(&Batch::getAttributeName, this),
				std::bind(&Batch::setAttributeName, this, std::placeholders::_1),
				Util::TypeOf<Batch>(), getName(), "AttributeName", "");
	SimulationControlGeneric<std::string>* propSize = new SimulationControlGeneric<std::string>(
				std::bind(&Batch::getBatchSize, this),
				std::bind(&Batch::setBatchSize, this, std::placeholders::_1),
				Util::TypeOf<Batch>(), getName(), "BatchSize", "");
	

	_parentModel->getControls()->insert(propBatchType);
    _parentModel->getControls()->insert(propRule);
    _parentModel->getControls()->insert(propGroupedAttribs);
	_parentModel->getControls()->insert(propGroupedEntity);
	_parentModel->getControls()->insert(propAttributeName);
	_parentModel->getControls()->insert(propSize);

	// setting properties
	_addSimulationControl(propBatchType);
    _addSimulationControl(propRule);
    _addSimulationControl(propGroupedAttribs);
	_addSimulationControl(propGroupedEntity);
	_addSimulationControl(propAttributeName);
	_addSimulationControl(propSize);
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

void Batch::setBatchType(Batch::BatchType batchType) {
	_batchType = batchType;
}

Batch::BatchType Batch::getBatchType() const {
	return _batchType;
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
	ModelDataManager* elements = _parentModel->getDataManager();
	ModelDataDefinition* queueByName = elements->getDataDefinition(Util::TypeOf<Queue>(), this->getName() + ".Queue");
	if (queueByName != nullptr) {
		_queue = dynamic_cast<Queue*>(queueByName);
	}
	ModelDataDefinition* groupByName = elements->getDataDefinition(Util::TypeOf<EntityGroup>(), this->getName() + ".EntiyGroup");
	if (groupByName != nullptr) {
		_entityGroup = dynamic_cast<EntityGroup*>(groupByName);
	}
	if (_queue == nullptr || _entityGroup == nullptr) {
		_createInternalAndAttachedData();
		if (_queue == nullptr || _entityGroup == nullptr) {
			traceError("Batch internal data is not initialized (Queue/EntityGroup).", TraceManager::Level::L1_errorFatal);
			return;
		}
	}
	// Queue the incoming entity first, then evaluate whether a complete batch can be formed.
	double tnow = 0.0;
	if (_parentModel->getSimulation() != nullptr) {
		tnow = _parentModel->getSimulation()->getSimulatedTime();
	}
	_queue->insertElement(new Waiting(entity, tnow, this, 0));
	unsigned int batchSize = 0;
	try {
		batchSize = static_cast<unsigned int>(std::stoul(_batchSize));
	} catch (...) {
		batchSize = static_cast<unsigned int>(_parentModel->parseExpression(_batchSize));
	}
	if (batchSize == 0) {
		traceError("Batch size evaluated to zero. Entity will remain waiting.", TraceManager::Level::L3_errorRecover);
		return;
	}
	std::list<Waiting*>* entitiesToGroup = nullptr;
	// check if batch size is complete
	if (_rule == Batch::Rule::Any) {
		if (_queue->size() >= batchSize) {
			entitiesToGroup = new std::list<Waiting*>();
			for (unsigned int i = 0; i < batchSize; i++) {
				entitiesToGroup->insert(entitiesToGroup->end(), _queue->getAtRank(i));
			}
			traceSimulation(this, "Queue has " + std::to_string(_queue->size()) + " elements and a group with " + std::to_string(batchSize) + " first arrivals will be created",TraceManager::Level::L7_internal);

		} else {
			traceSimulation(this, "Queue has " + std::to_string(_queue->size()) + " elements, not enought to form a group with " + std::to_string(batchSize));
		}
	} else if (_rule == Batch::Rule::ByAttribute) {// rule IS Batch::Rule::ByAttribute
		// Track the first attribute value that reaches the target cardinality in queue order.
		std::map<double, unsigned int> countByValue;
		double selectedValue = 0.0;
		bool foundValue = false;
		for (unsigned int i = 0; i < _queue->size(); i++) {
			Waiting* waiting = _queue->getAtRank(i);
			double value = waiting->getEntity()->getAttributeValue(_attributeName);
			unsigned int count = ++countByValue[value];
			if (count >= batchSize) {
				selectedValue = value;
				foundValue = true;
				break;
			}
		}
		if (foundValue) {
			entitiesToGroup = new std::list<Waiting*>();
			for (unsigned int i = 0; i < _queue->size() && entitiesToGroup->size() < batchSize; i++) {
				Waiting* we = _queue->getAtRank(i);
				double testValue = we->getEntity()->getAttributeValue(_attributeName);
				if (testValue == selectedValue) {
					entitiesToGroup->insert(entitiesToGroup->end(), we);
				}
			}
			traceSimulation(this,  "Found at least " + std::to_string(batchSize) + " elements in queue with value (" + std::to_string(selectedValue) + ") for attribute \"" + _attributeName + "\", and a group will be created", TraceManager::Level::L7_internal);
		}
	} else { // BY EntityType

	}
	if (entitiesToGroup != nullptr) { // there are enought entities to group in
		//traceSimulation(this, "Queue has " + std::to_string(_queue->size()) + " elements, not enought to form a group");
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
			representativeEnt->setAttributeValue("Entity.Group", _entityGroup->getId()); // The "Entity.Group" attribute is the EntityGroup Id (an internal modeldatum of Batch), while the ID of the representative entity is the KEY of the map of that EntityGroup
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
			traceSimulation(this, "Group key " + std::to_string(groupIdKey) + " was created containing entities: " + txtEntsInGroup + " and representative entity wih attribute 'Entity.Group'=" + std::to_string(_entityGroup->getId()), TraceManager::Level::L7_internal);
		} else {
			traceSimulation(this, "Entity \"" + representativeEnt->getName() + "\" id=" + std::to_string(groupIdKey) + " now represented the removed entities: " + txtEntsInGroup, TraceManager::Level::L7_internal);
		}
		this->_parentModel->sendEntityToComponent(representativeEnt, this->getConnectionManager()->getFrontConnection());
	} else {
		if (_rule == Batch::Rule::Any) {
			traceSimulation(this, "Entity \"" + entity->getName() + "\" is waiting in the queue " + _queue->getName());
		} else if (_rule == Batch::Rule::ByAttribute) {
			traceSimulation(this, "Entity \"" + entity->getName() + "\" with attribute '" + _attributeName + "'=" + std::to_string(entity->getAttributeValue(_attributeName)) + " is waiting in the queue " + _queue->getName());
		} else { // by entity type
		}
	}
}

bool Batch::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		_batchType = static_cast<Batch::BatchType> (fields->loadField("batchType", static_cast<int> (DEFAULT.batchType)));
		_rule = static_cast<Batch::Rule> (fields->loadField("rule", static_cast<int> (DEFAULT.rule)));
		_groupedAttributes = static_cast<Batch::GroupedAttribs> (fields->loadField("groupedAttributes", static_cast<int> (DEFAULT.groupedAttributes)));
		_batchSize = fields->loadField("batchSize", DEFAULT.batchSize);
		_attributeName = fields->loadField("attributeName", DEFAULT.attributeName);
		std::string groupedEntityTypeName = fields->loadField("groupedEntityType", "");
		if (groupedEntityTypeName != "") {
			_groupedEntityType = dynamic_cast<EntityType*> (_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<EntityType>(), groupedEntityTypeName));
		}
	}
	return res;
}

void Batch::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
	fields->saveField("batchType", static_cast<int> (_batchType), static_cast<int> (DEFAULT.batchType), saveDefaultValues);
	fields->saveField("rule", static_cast<int> (_rule), static_cast<int> (DEFAULT.rule), saveDefaultValues);
	fields->saveField("groupedAttributes", static_cast<int> (_groupedAttributes), static_cast<int> (DEFAULT.groupedAttributes), saveDefaultValues);
	fields->saveField("batchSize", _batchSize, DEFAULT.batchSize, saveDefaultValues);
	fields->saveField("attributeName", _attributeName, DEFAULT.attributeName, saveDefaultValues);
	if (_groupedEntityType != nullptr) {
		fields->saveField("groupedEntityType", _groupedEntityType->getName(), "", saveDefaultValues);
	}
}

void Batch::_createInternalAndAttachedData() {
	// Keep attached-data keys stable across rechecks and remove stale aliases.
	_attachedAttributesInsert({"Entity.Group"});
	_attachedDataRemove("GroupdEntityType");
	if (_groupedEntityType != nullptr) {
		_attachedDataInsert("GroupedEntityType", _groupedEntityType);
	} else {
		_attachedDataRemove("GroupedEntityType");
	}
	if ((_queue == nullptr || _entityGroup == nullptr)) {
		PluginManager* plugins = _parentModel->getParentSimulator()->getPluginManager();
		if (_queue == nullptr) {
			_queue = plugins->newInstance<Queue>(_parentModel, this->getName() + ".Queue");
		}
		if (_queue == nullptr) {
			// Fallback for runtime contexts where plugin lookup is unavailable but Queue is linked in-process.
			_queue = new Queue(_parentModel, this->getName() + ".Queue");
		}
		if (_queue != nullptr) {
			ModelDataDefinition::CreateInternalData(_queue);
		}
		if (_entityGroup == nullptr) {
			_entityGroup = plugins->newInstance<EntityGroup>(_parentModel, this->getName() + ".EntiyGroup");
		}
		if (_entityGroup == nullptr) {
			// Fallback for runtime contexts where plugin lookup is unavailable but EntityGroup is linked in-process.
			_entityGroup = new EntityGroup(_parentModel, this->getName() + ".EntiyGroup");
		}
		if (_entityGroup != nullptr) {
			ModelDataDefinition::CreateInternalData(_entityGroup);
		}
	}
	if (_queue != nullptr) {
		_internalDataInsert("EntityQueue", _queue);
	} else {
		_internalDataRemove("EntityQueue");
	}
	if (_entityGroup != nullptr) {
		_internalDataInsert("EntityGroup", _entityGroup);
	} else {
		_internalDataRemove("EntityGroup");
	}
	if (_attributeName != "") {
		ModelDataManager* elements = _parentModel->getDataManager();
		ModelDataDefinition* attribute = elements->getDataDefinition(Util::TypeOf<Attribute>(), _attributeName);
		_attachedDataInsert("AttributeName", attribute);
	} else {
		_attachedDataRemove("AttributeName");
	}
}

bool Batch::_check(std::string& errorMessage) {
	// Validate expression and conditional references without mixing arithmetic operators on booleans.
	bool resultAll = true;
	ModelDataManager* elements = _parentModel->getDataManager();
	if (_groupedEntityType != nullptr) {
		resultAll &= elements->check(Util::TypeOf<EntityType>(), _groupedEntityType, "GroupedEntityType", errorMessage);
	}
	if (_rule == Batch::Rule::ByAttribute) {
		resultAll &= elements->check(Util::TypeOf<Attribute>(), _attributeName, "AttributeName", true, errorMessage);
	} else if (!_attributeName.empty()) {
		resultAll &= elements->check(Util::TypeOf<Attribute>(), _attributeName, "AttributeName", false, errorMessage);
	}
	resultAll &= _parentModel->checkExpression(_batchSize, "BatchSize", errorMessage);

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

void Batch::_createReportStatisticsDataDefinitions() {
}

void Batch::_createEditableDataDefinitions() {
}

void Batch::_createOthersDataDefinitions() {
}
