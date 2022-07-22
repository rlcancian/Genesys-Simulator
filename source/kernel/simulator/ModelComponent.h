/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   Element.h
 * Author: rafael.luiz.cancian
 *
 * Created on 21 de Junho de 2018, 15:56
 */

#ifndef MODELCOMPONENT_H
#define MODELCOMPONENT_H

#include <string>
#include <list>

#include "Plugin.h"
#include "../util/List.h"
#include "Entity.h"
#include "ModelDataDefinition.h"
#include "ConnectionManager.h"
//namespace GenesysKernel {

class Model;
class Event;

/*!
 * A component of the model is a block that represents a specific behavior to be simulated. The behavior is triggered when an entity arrives at the component, which corresponds to the occurrence of an event. A simulation model corresponds to a set of interconnected components to form the process by which the entity is submitted.
 * @param model The model this component belongs to
 */
class ModelComponent : public ModelDataDefinition {
public:
	ModelComponent(Model* model, std::string componentTypename, std::string name = "");
	virtual ~ModelComponent();
public:
	virtual std::string show();
public:
	ConnectionManager* getConnections() const; ///< Returns a list of components directly connected to the output. Usually the components have a single output, but they may have none (such as Dispose) or more than one (as Decide). In addition to the component, NextComponents specifies the inputPortNumber of the next component where the entity will be sent to. Ussually the components have a single input, but they may have none (such as Create) or more than one (as Match).
	bool hasBreakpointAt();
public: // static
	static void DispatchEvent(Event* event); ///< This method triggers the simulation of the behavior of the component. It is invoked when an event (corresponding to this component) is taken from the list of future events or when an entity arrives at this component by connection.
	//static void InitBetweenReplications(ModelComponent* component);
	static void CreateInternalData(ModelComponent* component);
	static bool Check(ModelComponent* component);
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	static void SaveInstance(PersistenceRecord *fields, ModelComponent* component);
	void setDescription(std::string _description);
	std::string getDescription() const;
protected: // must be overriden by derived classes
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues);
	virtual bool _loadInstance(PersistenceRecord *fields);
	// new virtual methods
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) = 0;
protected: // could be overriden by derived classes.
	//virtual bool _check(std::string* errorMessage);
	//virtual void _initBetweenReplications();
	//virtual void _createInternalAndAttachedData();
	//virtual ParserChangesInformation* _getParserChangesInformation();
protected:

	const struct DEFAULT_VALUES {
		unsigned int nextSize = 1;
		unsigned int nextinputPortNumber = 0;
		std::string description = "";
	} DEFAULT;
	std::string _description = DEFAULT.description;
	ConnectionManager* _connections = new ConnectionManager();
};
//namespace\\}
#endif /* MODELCOMPONENT_H */

