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
#include "List.h"
#include "Entity.h"
#include "ModelElement.h"
#include "ConnectionManager.h"
//namespace GenesysKernel {

class Model;

/**
 * A component of the model is a block that represents a specific behavior to be simulated. The behavior is triggered when an entity arrives at the component, which corresponds to the occurrence of an event. A simulation model corresponds to a set of interconnected components to form the process by which the entity is submitted.
 * @param model The model this component belongs to
 */
class ModelComponent : public ModelElement {
public:
	ModelComponent(Model* model, std::string componentTypename, std::string name = "");
	virtual ~ModelComponent();
public:
	virtual std::string show();
public:
	ConnectionManager* getNextComponents() const; ///< Returns a list of components directly connected to the output. Usually the components have a single output, but they may have none (such as Dispose) or more than one (as Decide). In addition to the component, NextComponents specifies the inputNumber of the next component where the entity will be sent to. Ussually the components have a single input, but they may have none (such as Create) or more than one (as Match).
public: // static
	static void Execute(Entity* entity, ModelComponent* component, unsigned int inputNumber); ///< This method triggers the simulation of the behavior of the component. It is invoked when an event (corresponding to this component) is taken from the list of future events or when an entity arrives at this component by connection.
	//static void InitBetweenReplications(ModelComponent* component);
	static void CreateInternalElements(ModelComponent* component);
	static bool Check(ModelComponent* component);
	static ModelComponent* LoadInstance(Model* model, std::map<std::string, std::string>* fields);
	static std::map<std::string, std::string>* SaveInstance(ModelComponent* component);
    void setDescription(std::string _description);
    std::string getDescription() const;
protected: // pure virtual methods
	virtual void _execute(Entity* entity) = 0;
protected: // virtual methods
	virtual std::map<std::string, std::string>* _saveInstance();
	virtual bool _loadInstance(std::map<std::string, std::string>* fields);
	virtual void _createInternalElements(); ///< This method is necessary only for those components that instantiate internal elements that must exist before simulation starts and even before model checking. That's the case of components that have internal StatisticsCollectors, since others components may refer to them as expressions (as in "TVAG(ThisCSTAT)") and therefore the element must exist when checking such expression 
protected:
	const struct DEFAULT_VALUES {
		unsigned int nextSize = 1;
		unsigned int nextInputNumber = 0;
		std::string description = "";
	} DEFAULT;
	std::string _description = DEFAULT.description;
	ConnectionManager* _connections = new ConnectionManager();
};
//namespace\\}
#endif /* MODELCOMPONENT_H */

