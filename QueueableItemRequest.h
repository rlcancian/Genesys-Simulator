/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   QueueableItemRequest.h
 * Author: rlcancian
 *
 * Created on 23 de abril de 2021, 15:09
 */

#ifndef QUEUEABLEITEMREQUEST_H
#define QUEUEABLEITEMREQUEST_H

#include "Queue.h"
#include "Set.h"
#include "ComponentManager.h"

// \todo should inhere from a common base to SeizeableItemRequest
class QueueableItemRequest {
public:

	enum class QueueableType : int {
		QUEUE = 1, SET = 2, HOLD = 3
	};

public:
	QueueableItemRequest(ModelElement* queueOrSet, QueueableItemRequest::QueueableType queueableType = QueueableItemRequest::QueueableType::QUEUE, std::string index = "0");
public: 
	//virtual bool _loadInstance(std::map<std::string, std::string>* fields, unsigned int parentIndex);
	//virtual std::map<std::string, std::string>* _saveInstance(unsigned int parentIndex);
	bool loadInstance(std::map<std::string, std::string>* fields);
	std::map<std::string, std::string>* saveInstance();
public:
	std::string show();
	void setIndex(std::string index);
	std::string getIndex() const;
	std::string getQueueableName() const;
	void setQueue(Queue* resource);
	Queue* getQueue() const;
	void setSet(Set* set);
	Set* getSet() const;
	//
	void setQueueableType(QueueableType queueableType);
	QueueableType getQueueableType() const;
    ModelElement* getQueueable() const;
    void setComponentManager(ComponentManager* _componentManager);

private:

	const struct DEFAULT_VALUES {
		const std::string quantityExpression = "1";
		const QueueableItemRequest::QueueableType queueableType = QueueableItemRequest::QueueableType::QUEUE;
		const std::string saveAttribute = "";
		const std::string index = "0";
	} DEFAULT;

	QueueableType _queueableType;
	ModelElement* _queueOrSet;
	std::string _queueableName;
	std::string _index;
private:
	ComponentManager* _componentManager;
};


#endif /* QUEUEABLEITEMREQUEST_H */

