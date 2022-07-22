/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   DropOff.h
 * Author: rlcancian
 *
 * Created on 03 de Junho de 2019, 15:15
 */

#ifndef DROPOFF_H
#define DROPOFF_H

#include "../../kernel/simulator/ModelComponent.h"

/*!
Dropoff module
DESCRIPTION
The Dropoff module removes a specified number of entities from the entity’s group
and sends them to another module, as specified by a graphical connection. Group
user-defined attribute value and internal attributes may be given to the dropped-off
entities based on a specified rule.
TYPICAL USES
* Loading shelves with product
* Separating a form for use in various departments
PROMPTS
Prompt Description
Name Unique module identifier displayed on the module shape.
Quantity Number of entities that will be dropped off from an incoming
representative grouped entity.
Starting Rank Starting rank of the entities to be dropped off, based on the
entities in the group.
Member Attributes Method of determining how to assign the representative entity
attribute values (other than costs/times) to the dropped-off
original entities.
Attribute Name Name of representative entity attribute(s) assigned to droppedoff original entities of the group
 */
class DropOff : public ModelComponent {
public: // constructors
	DropOff(Model* model, std::string name = "");
	virtual ~DropOff() = default;
public: // virtual
	virtual std::string show();
public: // static
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
protected: // virtual
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber);
	virtual bool _loadInstance(PersistenceRecord *fields);
protected: // virtual
	//virtual void _initBetweenReplications();
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues);
	virtual bool _check(std::string* errorMessage);
private: // methods
private: // attributes 1:1
private: // attributes 1:n
};


#endif /* DROPOFF_H */

