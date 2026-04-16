/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   Queue.cpp
 * Author: rafael.luiz.cancian
 *
 * Created on 21 de Agosto de 2018, 17:12
 */

#include "Queue.h"
#include "../../kernel/simulator/Model.h"
#include "../../kernel/simulator/Attribute.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &Queue::GetPluginInformation;
}
#endif

ModelDataDefinition* Queue::NewInstance(Model* model, std::string name) {
	return new Queue(model, name);
}

std::string Queue::convertEnumToStr(OrderRule rule) {
	switch (static_cast<int> (rule)) {
		case 0: return "FIFO";
		case 1: return "LIFO";
		case 2: return "HIGHESTVALUE";
		case 3: return "SMALLESTVALUE";
	}
	return "Unknown";
}

Queue::Queue(Model* model, std::string name) : ModelDataDefinition(model, Util::TypeOf<Queue>(), name) {
	//controls
	SimulationControlGeneric<std::string>* propAttributeName = new SimulationControlGeneric<std::string>(
				std::bind(&Queue::getAttributeName, this),
				std::bind(&Queue::setAttributeName, this, std::placeholders::_1),
				Util::TypeOf<Queue>(), getName(), "AttributeName", "");
	SimulationControlGenericEnum<Queue::OrderRule, Queue>* propOrderRule = new SimulationControlGenericEnum<Queue::OrderRule, Queue>(
                std::bind(&Queue::getOrderRule, this),
                std::bind(&Queue::setOrderRule, this, std::placeholders::_1),
                Util::TypeOf<Queue>(), getName(), "OrderRule", "");
	SimulationControlGeneric<int>* propOrderRuleInt = new SimulationControlGeneric<int>(
				std::bind(&Queue::getOrderRuleInt, this),
				std::bind(&Queue::setOrderRuleInt, this, std::placeholders::_1),
				Util::TypeOf<Queue>(), getName(), "OrderRuleInt", "");

	_parentModel->getControls()->insert(propAttributeName);
	_parentModel->getControls()->insert(propOrderRule);
	_parentModel->getControls()->insert(propOrderRuleInt);

	// setting properties
    _addProperty(propAttributeName);
    _addProperty(propOrderRule);
	_addProperty(propOrderRuleInt);

	_configureListComparator();
}

Queue::~Queue() {
	for (Waiting* waiting : *_list->list()) {
		delete waiting;
	}
	_list->clear();
	delete _list;
	//_parentModel->elements()->remove(Util::TypeOf<StatisticsCollector>(), _cstatNumberInQueue);
	//_parentModel->elements()->remove(Util::TypeOf<StatisticsCollector>(), _cstatTimeInQueue);
}

std::string Queue::show() {
	return ModelDataDefinition::show() +
			",waiting=" + this->_list->show();
}

void Queue::insertElement(Waiting* modeldatum) {
	modeldatum->setArrivalOrder(_nextArrivalOrder++);
	if (_reportStatistics) {
		double tnow = 0.0;
		if (_parentModel->getSimulation() != nullptr) {
			tnow = _parentModel->getSimulation()->getSimulatedTime();
		}
		double duration = tnow - _lastTimeNumberInQueueChanged;
		this->_cstatNumberInQueue->getStatistics()->getCollector()->addValue(_list->size(), duration); // save the OLD quantity and for how long it was there
		_lastTimeNumberInQueueChanged = tnow;
	}
	_list->insert(modeldatum);
}

void Queue::removeElement(Waiting* modeldatum) {
	if (modeldatum == nullptr) {
		return;
	}
	if (_reportStatistics) {
		double tnow = 0.0;
		if (_parentModel->getSimulation() != nullptr) {
			tnow = _parentModel->getSimulation()->getSimulatedTime();
		}
		double duration = tnow - _lastTimeNumberInQueueChanged;
		this->_cstatNumberInQueue->getStatistics()->getCollector()->addValue(_list->size(), duration); // save the OLD quantity and for how long it was there
		_lastTimeNumberInQueueChanged = tnow;
		double timeInQueue = tnow - modeldatum->getTimeStartedWaiting();
		this->_cstatTimeInQueue->getStatistics()->getCollector()->addValue(timeInQueue);
	}
	_list->remove(modeldatum);
	delete modeldatum;
}

void Queue::_initBetweenReplications() {
	for (Waiting* waiting : *_list->list()) {
		delete waiting;
	}
	this->_list->clear();
	_lastTimeNumberInQueueChanged = 0.0;
	_nextArrivalOrder = 0;
}

unsigned int Queue::size() {
	return _list->size();
}

Waiting* Queue::first() {
	if (_list->size() == 0) {
		return nullptr;
	}
	return _list->front();
}

Waiting* Queue::getAtRank(unsigned int rank) {
	return _list->getAtRank(rank);
}

void Queue::setAttributeName(std::string _attributeName) {
	this->_attributeName = _attributeName;
}

std::string Queue::getAttributeName() const {
	return _attributeName;
}

void Queue::setOrderRule(OrderRule _orderRule) {
	this->_orderRule = _orderRule;
}

Queue::OrderRule Queue::getOrderRule() const {
	return _orderRule;
}

void Queue::setOrderRuleInt(int orderRule){
	setOrderRule(static_cast<Queue::OrderRule>(orderRule));
}
int Queue::getOrderRuleInt() const{
	return static_cast<int>(_orderRule);
}


double Queue::sumAttributesFromWaiting(Util::identification attributeID) {
	double sum = 0.0;
	for (Waiting* waiting : *_list->list()) {
		sum += waiting->getEntity()->getAttributeValue(attributeID);
	}
	return sum;
}

double Queue::getAttributeFromWaitingRank(unsigned int rank, Util::identification attributeID) {
	Waiting* wait = _list->getAtRank(rank);
	if (wait != nullptr) {
		return wait->getEntity()->getAttributeValue(attributeID);
	}
	return 0.0;
}

PluginInformation* Queue::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Queue>(), &Queue::LoadInstance, &Queue::NewInstance);
	return info;
}

ModelDataDefinition* Queue::LoadInstance(Model* model, PersistenceRecord *fields) {
	Queue* newElement = new Queue(model);
	try {
		newElement->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newElement;
}

bool Queue::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelDataDefinition::_loadInstance(fields);
	if (res) {
		try {
			this->_attributeName = fields->loadField("attributeName", DEFAULT.attributeName);
			this->_orderRule = static_cast<OrderRule> (fields->loadField("orderRule", static_cast<int> (DEFAULT.orderRule)));
		} catch (...) {
		}
	}
	return res;
}

void Queue::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelDataDefinition::_saveInstance(fields, saveDefaultValues);
	fields->saveField("orderRule", static_cast<int> (this->_orderRule), static_cast<int> (DEFAULT.orderRule), saveDefaultValues);
	fields->saveField("attributeName", this->_attributeName, DEFAULT.attributeName, saveDefaultValues);
}

bool Queue::_check(std::string& errorMessage) {
	const bool attributeMandatory = _orderRule == OrderRule::HIGHESTVALUE || _orderRule == OrderRule::SMALLESTVALUE;
	return _parentModel->getDataManager()->check(Util::TypeOf<Attribute>(), _attributeName, "AttributeName", attributeMandatory, errorMessage);
}

void Queue::_createInternalAndAttachedData() {
	if (_reportStatistics) {
		if (_cstatNumberInQueue == nullptr) {
			_cstatNumberInQueue = new StatisticsCollector(_parentModel, getName() + "." + "NumberInQueue", this);
			_cstatTimeInQueue = new StatisticsCollector(_parentModel, getName() + "." + "TimeInQueue", this);
		}
		if (_cstatNumberInQueue != nullptr) {
			_internalDataInsert("NumberInQueue", _cstatNumberInQueue);
		}
		if (_cstatTimeInQueue != nullptr) {
			_internalDataInsert("TimeInQueue", _cstatTimeInQueue);
		}
	} else if (_cstatNumberInQueue != nullptr) {
		_internalDataClear();
	}
}

ParserChangesInformation * Queue::_getParserChangesInformation() {
	ParserChangesInformation* changes = new ParserChangesInformation();
	//changes->getProductionToAdd()->insert(...);
	//changes->getTokensToAdd()->insert(...);
	return changes;
}

void Queue::_configureListComparator() {
	_list->setSortFunc([this](const Waiting* a, const Waiting* b) {
		switch (_orderRule) {
			case Queue::OrderRule::FIFO:
				return a->getArrivalOrder() < b->getArrivalOrder();
			case Queue::OrderRule::LIFO:
				return a->getArrivalOrder() > b->getArrivalOrder();
			case Queue::OrderRule::HIGHESTVALUE:
			case Queue::OrderRule::SMALLESTVALUE: {
				if (_attributeName.empty()) {
					return a->getArrivalOrder() < b->getArrivalOrder();
				}
				Attribute* attribute = dynamic_cast<Attribute*> (_parentModel->getDataManager()->getDataDefinition(Util::TypeOf<Attribute>(), _attributeName));
				if (attribute == nullptr) {
					return a->getArrivalOrder() < b->getArrivalOrder();
				}
				const Util::identification attributeId = attribute->getId();
				const double valueA = a->getEntity()->getAttributeValue(attributeId);
				const double valueB = b->getEntity()->getAttributeValue(attributeId);
				if (valueA == valueB) {
					return a->getArrivalOrder() < b->getArrivalOrder();
				}
				if (_orderRule == Queue::OrderRule::HIGHESTVALUE) {
					return valueA > valueB;
				}
				return valueA < valueB;
			}
			default:
				return a->getArrivalOrder() < b->getArrivalOrder();
		}
	});
}
