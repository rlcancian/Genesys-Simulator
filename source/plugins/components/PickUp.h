/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   PickUp.h
 * Author: rlcancian
 *
 * Created on 03 de Junho de 2019, 15:15
 */

#ifndef PICKUP_H
#define PICKUP_H

#include "../../kernel/simulator/ModelComponent.h"

/*!
Pickup module
DESCRIPTION
The Pickup module removes a number of consecutive entities from a given queue
starting at a specified rank in the queue. The entities that are picked up are added to
the end of the incoming entity’s group.
TYPICAL USES
* Gathering an order from various queue locations
* Gathering completed forms for an office order
* Picking up students at a bus stop for school
PROMPTS
Prompt Description
Name Unique module identifier displayed on the module shape.
Quantity Number of entities to pick up.
Queue Name Name of the queue from which the entities will be picked up,
starting at the specified rank.
Starting Rank Starting rank of the entities to pick up from the queue, Queue
Name.
 */
class PickUp : public ModelComponent {
public: // constructors
	PickUp(Model* model, std::string name = "");
	virtual ~PickUp() = default;
public: // virtual
	virtual std::string show();
public: // static
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
protected: // virtual
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber);
	virtual bool _loadInstance(PersistenceRecord *fields);
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues);
protected: // virtual
	//virtual void _initBetweenReplications();
	virtual bool _check(std::string* errorMessage);
private: // methods
private: // attributes 1:1
private: // attributes 1:n
};


#endif /* PICKUP_H */

