/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Assign.h
 * Author: rafael.luiz.cancian
 *
 * Created on 31 de Agosto de 2018, 10:10
 */

#ifndef ASSIGN_H
#define ASSIGN_H

#include "../../kernel/simulator/ModelComponent.h"
#include "../../kernel/simulator/Model.h"
#include "../../kernel/simulator/Plugin.h"
#include "../../kernel/simulator/Attribute.h"
#include "../data/AssignmentItem.h"
#include "../data/Variable.h"

/*!
 Assign module
DESCRIPTION
This module is used for assigning new values to variables, entity attributes, entity
types, entity pictures, or other system variables. Multiple assignments can be made
with a single Assign module.
TYPICAL USES
* Accumulate the number of subassemblies added to a part
* Change an entity’s type to represent the customer copy of a multi-page form
* Establish a customer’s priority
PROMPTS
 Prompt Description
Name Unique module identifier displayed on the module shape.
Assignments Specifies the one or more assignments that will be made when an
entity executes the module.
Type Type of assignment to be made. Other can include system
variables, such as resource capacity or simulation end time.
Variable Name Name of the variable that will be assigned a new value when an
entity enters the module. Applies only when Type is Variable,
Variable Array (1D), or Variable Array (2D).
Row Specifies the row index for a variable array.
Column Specifies the column index for a variable array.
Attribute Name Name of the entity attribute that will be assigned a new value
when the entity enters the module. Applies only when Type is
Attribute.
Entity Type New entity type that will be assigned to the entity when the
entity enters the module. Applies only when Type is Entity Type.
Entity Picture New entity picture that will be assigned to the entity when the
entity enters the module. Applies only when Type is Entity
Picture.
Other Identifies the special system variable that will be assigned a new
value when an entity enters the module. Applies only when Type
is Other.
New Value Assignment value of the attribute, variable, or other system
variable. Does not apply when Type is Entity Type or Entity
Picture.
 */
class Assign : public ModelComponent {
public:
	Assign(Model* model, std::string name = "");
	virtual ~Assign() = default;
public:
	virtual std::string show() override;
public:
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
public:
	List<Assignment*>* getAssignments() const;
	void addAssignment(Assignment* newAssignment);
	void removeAssignment(Assignment* assignment);
protected: // must be overriden 
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override;
protected: // could be overriden by derived classes
	virtual bool _check(std::string& errorMessage) override;
	/*! This method returns all changes in the parser that are needed by plugins of this ModelDatas. When connecting a new plugin, ParserChangesInformation are used to change parser source code, whch is after compiled and dinamically linked to to simulator kernel to reflect the changes */
	//virtual ParserChangesInformation* _getParserChangesInformation();
	//virtual void _initBetweenReplications();
	/*! This method is necessary only for those components that instantiate internal elements that must exist before simulation starts and even before model checking. That's the case of components that have internal StatisticsCollectors, since others components may refer to them as expressions (as in "TVAG(ThisCSTAT)") and therefore the modeldatum must exist before checking such expression */
	virtual void _createInternalAndAttachedData(); /*< A ModelDataDefinition or ModelComponent that includes (internal) ou refers to (attach) other ModelDataDefinition must register them inside this method. */
	//virtual void _addProperty(PropertyBase* property);
private:
private:

	const struct DEFAULT_VALUES {
		const unsigned int assignmentsSize = 1;
	} DEFAULT;
	List<Assignment*>* _assignments = new List<Assignment*>();
};

#endif /* ASSIGN_H */

