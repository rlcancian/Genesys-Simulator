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

#include "SinkModelComponent.h"
#include "Counter.h"
#include "Plugin.h"

/*!
Dispose module
DESCRIPTION
This module is intended as the ending point for entities in a simulation model. Entity
statistics may be recorded before the entity is disposed of.
TYPICAL USES
 Parts leaving the modeled facility
 The termination of a business process
 Customers departing from the store
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

#endif /* DISPOSE_H */

