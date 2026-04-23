/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   QueueableItem.cpp
 * Author: rlcancian
 *
 * Created on 23 de abril de 2021, 15:09
 */

#include "plugins/components/DiscreteProcessing/auxiliar/QueueableItem.h"
#include "kernel/simulator/ModelDataDefinition.h"
#include "kernel/simulator/Model.h"
#include "kernel/simulator/Simulator.h"
#include <cassert>


std::string QueueableItem::convertEnumToStr(QueueableType type) {
	switch (static_cast<int> (type)) {
		case 0: return "QUEUE";
		case 1: return "SET";
		// case 2: return "HOLD";
	}
	return "Unknown";
}

void QueueableItem::_ensureSimulationControls(Model* model) {
	if (model == nullptr || _simulationControls == nullptr || _simulationControls->size() > 0) {
		return;
	}

	_modeldataManager = model->getDataManager();

	auto* propIndex = new SimulationControlGeneric<std::string>(
		std::bind(&QueueableItem::getIndex, this), std::bind(&QueueableItem::setIndex, this, std::placeholders::_1),
		Util::TypeOf<QueueableItem>(), getName(), "Index", "");
	auto* propQueue = new SimulationControlGenericClass<Queue*, Model*, Queue>(
		model,
		std::bind(&QueueableItem::getQueue, this), std::bind(&QueueableItem::setQueue, this, std::placeholders::_1),
		Util::TypeOf<QueueableItem>(), "", "Queue", "");
	auto* propSet = new SimulationControlGenericClass<Set*, Model*, Set>(
		model,
		std::bind(&QueueableItem::getSet, this), std::bind(&QueueableItem::setSet, this, std::placeholders::_1),
		Util::TypeOf<QueueableItem>(), "", "Set", "");
	auto* propType = new SimulationControlGenericEnum<QueueableItem::QueueableType, QueueableItem>(
		std::bind(&QueueableItem::getQueueableType, this),
		std::bind(&QueueableItem::setQueueableType, this, std::placeholders::_1),
		Util::TypeOf<QueueableItem>(), getName(), "QueueableType", "");

	model->getControls()->insert(propIndex);
	model->getControls()->insert(propQueue);
	model->getControls()->insert(propSet);
	model->getControls()->insert(propType);

	_addSimulationControl(propIndex);
	_addSimulationControl(propQueue);
	_addSimulationControl(propSet);
	_addSimulationControl(propType);
}

QueueableItem::QueueableItem(ModelDataDefinition* queueOrSet, QueueableItem::QueueableType queueableType, std::string index) {
	_queueableType = queueableType;
	_queueOrSet = queueOrSet;
	_queueableName = queueOrSet != nullptr ? queueOrSet->getName() : "";
	_index = index;
	if (Set* set = dynamic_cast<Set*>(queueOrSet)) {
		// A QueueableItem operates on queues; this contextual contract tells the property
		// editor that newly-created Set members should be Queue objects.
		set->setAllowedElementTypes({Util::TypeOf<Queue>()});
		set->setSetOfType(Util::TypeOf<Queue>());
	}
	if (queueOrSet != nullptr) {
		_modeldataManager = queueOrSet->getParentModel()->getDataManager();
		_ensureSimulationControls(queueOrSet->getParentModel());
	}
}

QueueableItem::QueueableItem(Model* model, std::string queueName = "") {
	_queueableType = QueueableItem::QueueableType::QUEUE;
	if (!queueName.empty()) {
		ModelDataDefinition* data = model->getDataManager()->getDataDefinition(Util::TypeOf<Queue>(), queueName);
		if (data != nullptr) {
			_queueOrSet = dynamic_cast<Queue*> (data);
		} else {
			_queueOrSet = model->getParentSimulator()->getPluginManager()->newInstance<Queue>(model, queueName);
		}
	} else {
		_queueOrSet = nullptr;
	}
	_index = "0";
    _queueableName = queueName;
	_modeldataManager = model != nullptr ? model->getDataManager() : nullptr;
	_ensureSimulationControls(model);
}

bool QueueableItem::loadInstance(PersistenceRecord *fields) {
	bool res = true;
	try {
		_queueableType = static_cast<QueueableItem::QueueableType> (fields->loadField("queueableType", static_cast<int> (DEFAULT.queueableType)));
		_queueableName = fields->loadField("queueable", "");
		_index = fields->loadField("index", DEFAULT.index);
		if (_modeldataManager != nullptr) {
			if (_queueableType == QueueableItem::QueueableType::QUEUE) {
				_queueOrSet = _modeldataManager->getDataDefinition(Util::TypeOf<Queue>(), _queueableName);
			} else if (_queueableType == QueueableItem::QueueableType::SET) {
				_queueOrSet = _modeldataManager->getDataDefinition(Util::TypeOf<Set>(), _queueableName);
			}
			if (_queueOrSet == nullptr) {
				auto model = _modeldataManager->getParentModel();
				if (_queueableType == QueueableItem::QueueableType::SET) {
					_queueOrSet = model->getParentSimulator()->getPluginManager()->newInstance<Set>(model, _queueableName);
				} else {
					_queueOrSet = model->getParentSimulator()->getPluginManager()->newInstance<Queue>(model, _queueableName);
				}
			}
			if (Set* set = dynamic_cast<Set*>(_queueOrSet)) {
				// Loading a QueueableItem re-applies the same Set member-type contract used by
				// the editor, so empty loaded Sets still know they should create Queue members.
				set->setAllowedElementTypes({Util::TypeOf<Queue>()});
				set->setSetOfType(Util::TypeOf<Queue>());
			}
			_ensureSimulationControls(_modeldataManager->getParentModel());
		}
		assert(_queueOrSet != nullptr);
	} catch (...) {
		res = false;
	}
	return res;
}

void QueueableItem::saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	fields->saveField("queueableType", static_cast<int> (_queueableType), static_cast<int> (DEFAULT.queueableType), saveDefaultValues);
	fields->saveField("queueable", _queueOrSet != nullptr ? _queueOrSet->getName() : "");
	fields->saveField("index", _index, DEFAULT.index, saveDefaultValues);
}

std::string QueueableItem::show() {
	return "queueType=" + std::to_string(static_cast<int> (_queueableType)) + ",queue=\"" + (_queueOrSet != nullptr ? _queueOrSet->getName() : "") + "\",index=\"" + _index + "\"";
}

void QueueableItem::_addSimulationControl(SimulationControl* control) {
	_simulationControls->insert(control);
}

List<SimulationControl*>* QueueableItem::getSimulationControls() const {
	return _simulationControls;
}

void QueueableItem::setIndex(std::string index) {
	this->_index = index;
}

std::string QueueableItem::getIndex() const {
	return _index;
}

std::string QueueableItem::getQueueableName() const {
	return _queueableName;
}

std::string QueueableItem::getName() const {
    return _queueableName.empty() ? "<none>" : _queueableName;
}

void QueueableItem::setQueue(Queue* queue) {
	this->_queueOrSet = queue;
	_queueableType = QueueableType::QUEUE;
	_queueableName = queue != nullptr ? queue->getName() : "";
	if (_modeldataManager == nullptr && queue != nullptr) {
		_modeldataManager = queue->getParentModel()->getDataManager();
	}
	if (_simulationControls != nullptr && _simulationControls->size() == 0 && _modeldataManager != nullptr) {
		_ensureSimulationControls(_modeldataManager->getParentModel());
	}
}

Queue* QueueableItem::getQueue() const {
	return dynamic_cast<Queue*> (_queueOrSet);
}

void QueueableItem::setSet(Set* set) {
	this->_queueOrSet = set;
	_queueableType = QueueableType::SET;
	_queueableName = set != nullptr ? set->getName() : "";
	if (set != nullptr) {
		// The Set type remains configurable in the kernel, but this association provides the
		// default and allowed type expected by QueueableItem-specific GUI editing.
		set->setAllowedElementTypes({Util::TypeOf<Queue>()});
		set->setSetOfType(Util::TypeOf<Queue>());
		if (_modeldataManager == nullptr) {
			_modeldataManager = set->getParentModel()->getDataManager();
		}
	}
	if (_simulationControls != nullptr && _simulationControls->size() == 0 && _modeldataManager != nullptr) {
		_ensureSimulationControls(_modeldataManager->getParentModel());
	}
}

Set* QueueableItem::getSet() const {
	return dynamic_cast<Set*> (_queueOrSet);
}

void QueueableItem::setQueueableType(QueueableItem::QueueableType queueableType) {
	this->_queueableType = queueableType;
	if ((_queueableType == QueueableType::QUEUE && dynamic_cast<Queue*>(_queueOrSet) == nullptr)
		|| (_queueableType == QueueableType::SET && dynamic_cast<Set*>(_queueOrSet) == nullptr)) {
		_queueOrSet = nullptr;
		_queueableName = "";
	}
}

QueueableItem::QueueableType QueueableItem::getQueueableType() const {
	return _queueableType;
}

ModelDataDefinition* QueueableItem::getQueueable() const {
	return _queueOrSet;
}

void QueueableItem::setElementManager(ModelDataManager* modeldataManager) {
	_modeldataManager = modeldataManager;
}

//void QueueableItem::setComponentManager(ComponentManager* componentManager) {
//    this->_componentManager = componentManager;
//}
