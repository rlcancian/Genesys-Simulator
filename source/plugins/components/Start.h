/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Start.h
 * Author: rlcancian
 *
 * Created on 11 de Setembro de 2019, 13:15
 */

#ifndef START_H
#define START_H

#include "../../kernel/simulator/ModelComponent.h"

/*!
Start module
DESCRIPTION
The Start module changes the status of a conveyor from inactive to active. The
conveyor may have been deactivated from either the Stop module or by initially
being set to inactive at the start of the simulation. The velocity of the conveyor may
be changed permanently when the conveyor is started.
TYPICAL USES
* Start a bottling conveyor after scheduled maintenance
* Start a baggage claim conveyor when bags have arrived
PROMPTS
Prompt Description
Name Unique name of the module that will be displayed in the
flowchart.
Conveyor Name Name of the conveyor to start.
Velocity Speed of the conveyor once it begins to operate. This value will
change the speed of the conveyor permanently, until it is changed
in another module.
Units Velocity time units.
 */
class Start : public ModelComponent {
public: // constructors
	Start(Model* model, std::string name = "");
	virtual ~Start() = default;
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


#endif /* START_H */

