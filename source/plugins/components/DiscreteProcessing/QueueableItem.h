/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   QueueableItem.h
 * Author: rlcancian
 *
 * Created on 23 de abril de 2021, 15:09
 */

#ifndef QUEUEABLEITEM_H
#define QUEUEABLEITEM_H

#include "plugins/data/DiscreteProcessing/Queue.h"
#include "plugins/data/DiscreteProcessing/Set.h"
#include "kernel/simulator/ModelDataManager.h"

// @TODO should inhere from a common base to SeizeableItem

class QueueableItem {
public:

	enum class QueueableType : int {
		QUEUE = 0, SET = 1, num_elements = 2 //, HOLD = 2, num_elements = 3
	};
public:
	static std::string convertEnumToStr(QueueableType type);
public:
	QueueableItem(ModelDataDefinition* queueOrSet, QueueableItem::QueueableType queueableType = QueueableItem::QueueableType::QUEUE, std::string index = "0");
	QueueableItem(Model* model, std::string queueName);
public:
	bool loadInstance(PersistenceRecord *fields);
	void saveInstance(PersistenceRecord *fields, bool saveDefaultValues);
public:
	std::string show();
	void setIndex(std::string index);
	std::string getIndex() const;
	std::string getQueueableName() const;
    std::string getName() const;
	void setQueue(Queue* resource);
	Queue* getQueue() const;
	void setSet(Set* set);
	Set* getSet() const;
	//
	void setQueueableType(QueueableType queueableType);
	QueueableType getQueueableType() const;
	ModelDataDefinition* getQueueable() const;
	void setElementManager(ModelDataManager* _modeldataManager);
	//void setComponentManager(ComponentManager* _componentManager);
	List<PropertyBase*>* getSimulationControls() const;
    List<PropertyBase*>* getProperties() const;
	void _addSimulationControl(PropertyBase* control);
    void _addProperty(PropertyBase* property);

	std::string getTypeDC() {return _typeDC;};
private:

	const struct DEFAULT_VALUES {
		const std::string quantityExpression = "1";
		const QueueableItem::QueueableType queueableType = QueueableItem::QueueableType::QUEUE;
		const std::string saveAttribute = "";
		const std::string index = "0";
	} DEFAULT;

	QueueableType _queueableType = DEFAULT.queueableType;
	ModelDataDefinition* _queueOrSet = nullptr;
	std::string _queueableName;
	std::string _index = DEFAULT.index;
	std::string _typeDC = Util::TypeOf<Queue>();
private:
	//ComponentManager* _componentManager;
	ModelDataManager* _modeldataManager = nullptr;
	List<PropertyBase*>* _simulationControls = new List<PropertyBase*>();
};


#endif /* QUEUEABLEITEM_H */
