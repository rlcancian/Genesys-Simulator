/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Create.h
 * Author: rafael.luiz.cancian
 *
 * Created on 21 de Junho de 2018, 20:12
 */

#ifndef CREATE_H
#define CREATE_H

#include <string>
#include <limits>
#include "SourceModelComponent.h"
#include "EntityType.h"
#include "Counter.h"
#include "Plugin.h"

/*!
 * Create is the most basic component to include the first entities into the model, and therefore is a source component (derived from SourceModelComponent)
Create module
DESCRIPTION
This module is intended as the starting point for entities in a simulation model.
Entities are created using a schedule or based on a time between arrivals. Entities
then leave the module to begin processing through the system. The entity type is
specified in this module.
TYPICAL USES
 The start of a part’s production in a manufacturing line
 A document’s arrival (for example, order, check, application) into a business
process
 A customer’s arrival at a service process (for example, retail store, restaurant,
information desk)
PROMPTS
Prompt Description
Name Unique module identifier displayed on the module shape.
Entity Type Name of the entity type to be generated.
Type Type of arrival stream to be generated. Types include Random
(uses an exponential distribution, user specifies mean), Schedule
(uses an exponential distribution, mean determined from the
specified Schedule module), Constant (user specifies constant
value; for example, 100), or Expression (drop-down list of
various distributions).
 Value Determines the mean of the exponential distribution (if Random
is used) or the constant value (if Constant is used) for the time
between arrivals. Applies only when Type is Random or
Constant.
Schedule Name Identifies the name of the schedule to be used. The schedule
defines the arrival pattern for entities arriving to the system.
Applies only when Type is Schedule.
Expression Any distribution or value specifying the time between arrivals.
Applies only when Type is Expression.
Units Time units used for interarrival and first creation times. Does not
apply when Type is Schedule.
Entities per Arrival Number of entities that will enter the system at a given time with
each arrival.
Max Arrivals Maximum number of entities that this module will generate.
When this value is reached, the creation of new entities by this
module ceases.
First Creation Starting time for the first entity to arrive into the system. Does
not apply when Type is Schedule.
 */
class Create : public SourceModelComponent {
public:
	Create(Model* model, std::string name = "");
	virtual ~Create() = default;
public:
	virtual std::string show();
public:
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, std::map<std::string, std::string>* fields);
protected:
	virtual void _execute(Entity* entity);
	virtual bool _loadInstance(std::map<std::string, std::string>* fields);
	virtual void _initBetweenReplications();
	virtual std::map<std::string, std::string>* _saveInstance();
	virtual bool _check(std::string* errorMessage);
	virtual void _createInternalElements();
private: // children elements
	Counter* _numberOut = nullptr;
};

#endif /* CREATE_H */
