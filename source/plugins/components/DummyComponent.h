/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Dummy.h
 * Author: rafael.luiz.cancian
 *
 * Created on 22 de Maio de 2019, 18:41
 */

#ifndef DUMMYCOMPONENT_H
#define DUMMYCOMPONENT_H

#include "../../kernel/simulator/ModelComponent.h"
#include "../data/DummyElement.h"

/*!
 This component ...
 */
class DummyComponent : public ModelComponent {
public: // constructors
	DummyComponent(Model* model, std::string name = "");
	virtual ~DummyComponent() = default;
public: // virtual
	virtual std::string show();
public: // static
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
protected: // must be overriden 
	virtual bool _loadInstance(PersistenceRecord *fields);
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues);
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber);
protected: // could be overriden .
	virtual bool _check(std::string* errorMessage);
	virtual void _initBetweenReplications();
	virtual void _createInternalAndAttachedData();
	//virtual ParserChangesInformation* _getParserChangesInformation();
private: // methods
private: // attributes 1:1

	const struct DEFAULT_VALUES {
		std::string someString = "Test";
		unsigned int someUint = 1;
	} DEFAULT;
	std::string _someString = DEFAULT.someString;
	unsigned int _someUint = DEFAULT.someUint;
	DummyElement* _internalDataDefinition = nullptr;
private: // attributes 1:n
};

#endif /* DUMMYCOMPONENT_H */

