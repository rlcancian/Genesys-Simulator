/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Search.h
 * Author: rlcancian
 *
 * Created on 03 de Junho de 2019, 15:20
 */

#ifndef SEARCH_H
#define SEARCH_H

#include "../../kernel/simulator/ModelComponent.h"

/*!
Search module
DESCRIPTION
The Search module searches a queue, a group (batch), or an expression to find the
entity rank (for entities in a queue or group) or the value of the global variable J that
satisfies the specified search condition. When searching a queue or group, the value
of the global system variable J is set to the rank of the first entity that satisfies Search
Condition, or to 0 if Search Condition is not satisfied. When searching an expression,
the global system variable J is set to the value of the first index value that satisfies the
search condition or to zero if no value of J in the specified range satisfies the search
condition.
When an entity arrives at a Search module, the index J is set to the starting index and
the search condition is then checked. If the search condition is satisfied, the search
ends and the current value of J is retained. Otherwise, the value of J is increased or
decreased and the condition is rechecked. This process repeats until the search
condition is satisfied or the ending value is reached. If the condition is not met or
there are no entities in the queue or group, J is set equal to 0.
TYPICAL USES
* Looking for a particular order number in a queue
* Searching a group for a certain part type
* Determining which process to enter based on availability of resources (search an
expression)
 Prompt Description
Name Unique module identifier displayed on the module shape.
Type Determination of what will be searched. Search options include
entities in a queue, entities within a group (batch) or some
expression(s).
Queue Name Name of the queue that will be searched. Applies only when the
Type is Search a Queue.
Starting Value Starting rank in the queue or group or starting value for J in an
expression.
Ending Value Ending rank in the queue or group or ending value for J in an
expression.
Search Condition Condition containing the index J for searching expressions or
containing an attribute name(s) for searching queues or batches.
 */
class Search : public ModelComponent {
public: // constructors
	Search(Model* model, std::string name = "");
	virtual ~Search() = default;
public: // virtual
	virtual std::string show();
public: // static
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
protected: // virtual
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber);
	virtual bool _loadInstance(PersistenceRecord *fields);
protected: // virtual
	//virtual void _initBetweenReplications();
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues);
	virtual bool _check(std::string* errorMessage);
private: // methods
private: // attributes 1:1
private: // attributes 1:n
};


#endif /* SEARCH_H */

