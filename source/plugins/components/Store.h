/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Store.h
 * Author: rlcancian
 *
 * Created on 11 de Setembro de 2019, 13:07
 */

#ifndef STORE_H
#define STORE_H

#include "../../kernel/simulator/ModelComponent.h"

/*!
Store module
DESCRIPTION
The Store module adds an entity to storage. The Unstore module may then be used to
remove the entity from the storage.
When an entity arrives at the Store module, the storage specified is incremented, and
the entity immediately moves to the next module in the model.
Storages are useful for displaying entity animation while an entity undergoes
processing in other modules. Additionally, statistics may be kept on the number of
entities in storage.
TYPICAL USES
* Animating a part through a number of delay operations (load, setup, process,
unload)
* Tracking the number of customers within a grocery store (place in storage upon
entry)
PROMPTS
Prompt Description
Name Unique module identifier displayed on the module shape.
Type Method of specifying the storage name as a Storage, Set,
Attribute, or Expression.
Storage Name Name of the storage to which the entity will be added. Applies
only when the Type is Storage.
Set Name Name of the storage set from which the storage is to be selected.
Applies only when the Type is Set.
Set Index Index into the defined storage set that contains the desired
storage name. Applies only when the Type is Set.
Attribute Name of the attribute whose value contains the storage. Applies
only when the Type is Attribute.
Expression Expression that is evaluated to the storage into which the entity
is placed. Applies only when the Type is Expression.
 */
class Store : public ModelComponent {
public: // constructors
	Store(Model* model, std::string name = "");
	virtual ~Store() = default;
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


#endif /* STORE_H */

