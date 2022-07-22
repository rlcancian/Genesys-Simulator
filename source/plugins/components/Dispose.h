/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Dispose.h
 * Author: rafael.luiz.cancian
 *
 * Created on 21 de Junho de 2018, 20:13
 */

#ifndef DISPOSE_H
#define DISPOSE_H

#include "../../kernel/simulator/SinkModelComponent.h"
#include "../../kernel/simulator/Counter.h"
#include "../../kernel/simulator/Plugin.h"

/*!
Dispose module
DESCRIPTION
This module is intended as the ending point for entities in a simulation model. Entity
statistics may be recorded before the entity is disposed of.
TYPICAL USES
* Parts leaving the modeled facility
* The termination of a business process
* Customers departing from the store
Prompt Description
Name Unique module identifier displayed on the module shape.
Record Entity Statistics Determines whether or not the incoming entity’s statistics will be
recorded. Statistics include value-added time, non-value-added
time, wait time, transfer time, other time, total time, value-added
cost, non-value-added cost, wait cost, transfer cost, other cost,
and total cost. 
 */
class Dispose : public SinkModelComponent {
public:
	Dispose(Model* model, std::string name = "");
	virtual ~Dispose() = default;
public:
	virtual std::string show();
public:
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
protected:
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber);
	virtual bool _loadInstance(PersistenceRecord *fields);
	virtual void _initBetweenReplications();
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues);
	virtual bool _check(std::string* errorMessage);
	virtual void _createInternalAndAttachedData();
private: // internel elements
	Counter* _numberOut = nullptr;
};

#endif /* DISPOSE_H */

