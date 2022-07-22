/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Stop.h
 * Author: rlcancian
 *
 * Created on 11 de Setembro de 2019, 13:15
 */

#ifndef STOP_H
#define STOP_H

#include "../../kernel/simulator/ModelComponent.h"

/*!
Stop module
DESCRIPTION
The Stop module sets the operational status of a conveyor to inactive. The conveyor
may have been activated from either the Start module or by initially being set to
active at the start of the simulation. When the entity enters the Stop module, the
conveyor will stop immediately, regardless of the type of conveyor or the number of
entities currently on the conveyor.
TYPICAL USES
* Stop a baggage conveyor after a pre-determined amount of time
* Stop a conveyor for scheduled maintenance
PROMPTS
Prompt Description
Name Unique name of the module that will be displayed in the
flowchart.
Conveyor Name Name of the conveyor to stop.
 */
class Stop : public ModelComponent {
public: // constructors
	Stop(Model* model, std::string name = "");
	virtual ~Stop() = default;
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


#endif /* STOP_H */

