/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Decide.h
 * Author: rafael.luiz.cancian
 *
 * Created on 9 de Agosto de 2018, 20:39
 */

#ifndef DECIDE_H
#define DECIDE_H

#include "../../kernel/simulator/ModelComponent.h"
#include "../../kernel/simulator/Counter.h"

/*!
 Decide module
DESCRIPTION
This module allows for decision-making processes in the system. It includes options
to make decisions based on one or more conditions (for example, if entity type is
Gold Card) or based on one or more probabilities (for example, 75%, true; 25%,
false). Conditions can be based on attribute values (for example, Priority), variable
values (for example, Number Denied), the entity type, or an expression (for example,
NQ(ProcessA.Queue)).
There are two exit points out of the Decide module when its specified type is either
2-way by Chance or 2-way by Condition. There is one exit point for “true” entities
and one for “false” entities. When the N-way by Chance or by Condition type is
specified, multiple exit points are shown for each condition or probability and a
single “else” exit. The number of entities that exit from each type (true/false) is
displayed for
2-way by Chance or by Condition modules only.
TYPICAL USES
* Dispatching a faulty part for rework
* Branching accepted vs. rejected checks
* Sending priority customers to a dedicated process
Prompt Description
Name Unique module identifier displayed on the module shape.
Type Indicates whether the decision is based on a condition (if X>Y)
or by chance/percentage (for example, 60%, yes; 40%, no). The
type can be specified as either 2-way or N-way. 2-way allows for
one condition or probability (plus the “false” exit). N-way allows
for any number of conditions or probabilities to be specified as
well as an “else” exit.
Conditions Defines one or more conditions used to direct entities to different
modules. Applies only when Type is N-way by Condition.
Percentages Defines one or more percentages used to direct entities to
different modules. Applies only when Type is N-way by Chance.
Percent True Value that will be checked to determine the percentage of entities
sent out a given True exit.
If Types of conditions that are available for evaluation: Variable,
Variable Array (1D), Variable Array (2D), Attribute, Entity
Type, Expression.
Named Specifies the name of the variable, attribute, or entity type that
will be evaluated when an entity enters the module. Does not
apply when Type is Expression.
Is Evaluator for the condition. Applies only to Attribute and
Variable conditions.
Row Specifies the row index for a variable array. Applies only when
Type is N-way by Condition or 2-way by Condition and Variable
is Array 1-D or Array 2-D.
Column Specifies the column index for a variable array. Applies only
when Type is N-way by Condition or 2-way by Condition and
Variable is Array 1-D or Array 2-D.
Value Expression that will be either compared to an attribute or
variable or that will be evaluated as a single expression to
determine if it is true or false. Does not apply to Entity Type
condition. If Type is Expression, this value must also include the
evaluator (for example, Color<>Red).
 */
class Decide : public ModelComponent {
public:
	Decide(Model* model, std::string name = "");
	virtual ~Decide() = default;
public:
	List<std::string>* getConditions() const;

public:
	virtual std::string show();
public:
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
protected:
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber);
	virtual bool _loadInstance(PersistenceRecord *fields);
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues);
protected:
	//virtual void _initBetweenReplications();
	virtual bool _check(std::string* errorMessage);
	virtual void _createInternalAndAttachedData();
private:
	List<std::string>* _conditions = new List<std::string>();
	List<Counter*>* _numberOuts = nullptr;
private:

};

#endif /* DECIDE_H */

