/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Exit.h
 * Author: rlcancian
 *
 * Created on 11 de Setembro de 2019, 13:15
 */

#ifndef EXIT_H
#define EXIT_H

#include "../../kernel/simulator/ModelComponent.h"

/*!
Exit module
DESCRIPTION
The Exit module releases the entity’s cells on the specified conveyor. If another entity
is waiting in queue for the conveyor at the same station when the cells are released, it
will then access the conveyor.
TYPICAL USES
* Cases exit a conveyor for packing
* Bad parts are removed from the conveyor and disposed
* Passengers remove luggage from the baggage claim conveyor
PROMPTS
Prompt Description
Name Unique name of the module that will be displayed in the
flowchart.
Conveyor Name Name of the conveyor on which the entity will exit. If left blank,
the previously accessed conveyor is assumed.
# of Cells Number of contiguous conveyor cells the entity will relinquish.
 */
class Exit : public ModelComponent {
public: // constructors
	Exit(Model* model, std::string name = "");
	virtual ~Exit() = default;
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


#endif /* EXIT_H */

