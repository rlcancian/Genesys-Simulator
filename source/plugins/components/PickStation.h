/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   PickStation.h
 * Author: rlcancian
 *
 * Created on 11 de Setembro de 2019, 13:08
 */

#ifndef PICKSTATION_H
#define PICKSTATION_H

#include "../../kernel/simulator/ModelComponent.h"

/*!
 PickStation module
DESCRIPTION
The PickStation module allows an entity to select a particular station from the
multiple stations specified. This module picks among the group of stations based on
the selection logic defined with the module. The entity may then route, transport,
convey, or connect to the station specified. If the method chosen is connect, the
selected station is assigned to an entity attribute. The station selection process is
based on the minimum or maximum value of a variety of system variables and
expressions.
TYPICAL USES
* A part sent to a processing station based on machine’s availability at each station
* A loan application sent to a set of loan officers based on the number sent to each
officer
* A customer selecting among cashier lines based on the least number waiting in
each line
PROMPTS
Prompt Description
Name Unique name of the module that will be displayed in the
flowchart.
Test Condition Test condition to use for the station selection process, either
Minimum or Maximum.
Number En Route to
Station
The number of entities transferring to the station is considered in
the station selection process.
Number in Queue The number of entities in the queue at the station is considered in
the station selection process.
Number of Resources
Busy
The number of busy resources at the station is considered in the
station selection process.
Expression Determines if an additional user-defined expression is
considered in the station selection process.
Transfer Type Determines how an entity will be transferred out of this module
to its next destination station—either Route, Convey,
Transport, or Connect.
Save Attribute Defines the name of the attribute that will store the station name
that is selected, visible when the transfer method is Connect.
Route Time Move time of the entity from its current station to the station
determined through this module.
Units Time units for route-time parameters.
 */
class PickStation : public ModelComponent {
public: // constructors
	PickStation(Model* model, std::string name = "");
	virtual ~PickStation() = default;
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


#endif /* PICKSTATION_H */

