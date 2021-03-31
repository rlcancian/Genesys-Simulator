/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Queue.h
 * Author: rafael.luiz.cancian
 *
 * Created on 21 de Agosto de 2018, 17:12
 */

#ifndef QUEUE_H
#define QUEUE_H

#include "ModelElement.h"
#include "List.h"
#include "Entity.h"
#include "ElementManager.h"
#include "StatisticsCollector.h"
#include "Plugin.h"
#include "ModelComponent.h"

class Waiting {
public:

	Waiting(Entity* entity, ModelComponent* component, double timeStartedWaiting) {
		_entity = entity;
		_component = component;
		_timeStartedWaiting = timeStartedWaiting;
	}

	virtual ~Waiting() = default;
public:

	virtual std::string show() {
		return //ModelElement::show()+
		",entity=" + std::to_string(_entity->getId()) +
				",component=\"" + _component->getName() + "\"" +
				",timeStatedWaiting=" + std::to_string(_timeStartedWaiting);
	}
public:

	double getTimeStartedWaiting() const {
		return _timeStartedWaiting;
	}

	ModelComponent* getComponent() const {
		return _component;
	}

	Entity* getEntity() const {
		return _entity;
	}
private:
	Entity* _entity;
	ModelComponent* _component;
	double _timeStartedWaiting;
};

/*!
 Queue module
DESCRIPTION
This data module may be utilized to change the ranking rule for a specified queue.
The default ranking rule for all queues is First In, First Out unless otherwise specified
in this module. There is an additional field that allows the queue to be defined as
shared.
TYPICAL USES
 Stack of work waiting for a resource at a Process module
 Holding area for documents waiting to be collated at a Batch module
Prompt Description
Name The name of the queue whose characteristics are being defined.
This name must be unique.
Type Ranking rule for the queue, which can be based on an attribute.
Types include First In, First Out; Last In, First Out; Lowest
Attribute Value (first); and Highest Attribute Value (first). A
low attribute value would be 0 or 1, while a high value may be
200 or 300.
Attribute Name Attribute that will be evaluated for the Lowest Attribute Value or
Highest Attribute Value types. Entities with lowest or highest
values of the attribute will be ranked first in the queue, with ties
being broken using the First In, First Out rule.
Shared Check box that determines whether a specific queue is used in
multiple places within the simulation model. Shared queues can
only be used for seizing resources (for example, with the Seize
module from the Advanced Process panel).
Report Statistics Specifies whether or not statistics will be collected automatically
and stored in the report database for this queue.
 */
class Queue : public ModelElement {
public:

	enum class OrderRule : int {
		FIFO = 1, LIFO = 2, HIGHESTVALUE = 3, SMALLESTVALUE = 4
	};

public:
	Queue(Model* model, std::string name = "");
	virtual ~Queue();
public:
	virtual std::string show();
public:
	static PluginInformation* GetPluginInformation();
	static ModelElement* LoadInstance(Model* model, std::map<std::string, std::string>* fields);
public:
	void insertElement(Waiting* element);
	void removeElement(Waiting* element);
	unsigned int size();
	Waiting* first();
	Waiting* getAtRank(unsigned int rank);
	void setAttributeName(std::string _attributeName);
	std::string getAttributeName() const;
	void setOrderRule(OrderRule _orderRule);
	Queue::OrderRule getOrderRule() const;
public: // to implement SIMAN functions
	double sumAttributesFromWaiting(Util::identification attributeID); // use to implement SIMAN SAQUE function
	double getAttributeFromWaitingRank(unsigned int rank, Util::identification attributeID);
public:
	void initBetweenReplications();
protected:
	virtual bool _loadInstance(std::map<std::string, std::string>* fields);
	virtual std::map<std::string, std::string>* _saveInstance();
	virtual bool _check(std::string* errorMessage);
	virtual void _createInternalElements();
	virtual ParserChangesInformation* _getParserChangesInformation();

private:
	void _initCStats();
private: //1::n
	List<Waiting*>* _list = new List<Waiting*>();
private: //1::1
	OrderRule _orderRule = OrderRule::FIFO;
	std::string _attributeName = "";
private: // inner children elements
	StatisticsCollector* _cstatNumberInQueue = nullptr;
	StatisticsCollector* _cstatTimeInQueue;
};

#endif /* QUEUE_H */

