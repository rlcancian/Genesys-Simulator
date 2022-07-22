/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Remove.h
 * Author: rlcancian
 *
 * Created on 03 de Junho de 2019, 15:20
 */

#ifndef REMOVE_H
#define REMOVE_H

#include "../../kernel/simulator/ModelComponent.h"

/*!
Remove module
DESCRIPTION
The Remove module removes a single entity from a specified position in a queue and
sends it to a designated module.
When an entity arrives at a Remove module, it removes the entity from the specified
queue and sends it to the connected module. The rank of the entity signifies the
location of the entity within the queue. The entity that caused the removal proceeds to
the next module specified and is processed before the removed entity.
TYPICAL USES
* Removing an order from a queue that is due to be completed next
* Calling a patient from a waiting room for an examination
* Retrieving the next order to be processed from a pile of documents
Prompt Description
Name Unique module identifier displayed on the module shape.
Queue Name Name of the queue from which the entity will be removed.
Rank of Entity Rank of the entity to remove from within the queue.
 */
class Remove : public ModelComponent {
public: // constructors
	Remove(Model* model, std::string name = "");
	virtual ~Remove() = default;
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


#endif /* REMOVE_H */

