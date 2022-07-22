/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Separate.h
 * Author: rlcancian
 *
 * Created on 03 de Junho de 2019, 15:14
 */

#ifndef SEPARATE_H
#define SEPARATE_H

#include "../../kernel/simulator/ModelComponent.h"

/*!
 Separate module
DESCRIPTION
This module can be used to either copy an incoming entity into multiple entities or to
split a previously batched entity. Rules for allocating costs and times to the duplicate
are also specified. Rules for attribute assignment to member entities are specified as
well.
When splitting existing batches, the temporary representative entity that was formed
is disposed and the original entities that formed the group are recovered. The entities
proceed sequentially from the module in the same order in which they originally were
added to the batch.
When duplicating entities, the specified number of copies is made and sent from the
module. The original incoming entity also leaves the module.
TYPICAL USES
* Send individual entities to represent boxes removed from a container
* Send an order both to fulfillment and billing for parallel processing
* Separate a previously batched set of documents
PROMPTS
Prompt Description
Name Unique module identifier displayed on the module shape.
Type Method of separating the incoming entity. Duplicate Original
will simply take the original entity and make some number of
identical duplicates. Split Existing Batch requires that the
incoming entity be a temporarily batched entity using the Batch
module. The original entities from the batch will be split.
Percent Cost to
Duplicates
Allocation of costs and times of the incoming entity to the
outgoing duplicates. This value is specified as a percentage of
the original entity’s costs and times (between 0-100). The
percentage specified will be split evenly between the duplicates,
while the original entity will retain any remaining cost/time
percentage. Visible only when Type is Duplicate Original.
# of Duplicates Number of outgoing entities that will leave the module, in
addition to the original incoming entity. Applies only when Type
is Duplicate Original.
Member Attributes Method of determining how to assign the representative entity
attribute values to the original entities.These options relate to six
of the special-purpose attributes (Entity.Type, Entity.Picture,
Entity.Sequence, Entity.Station, Entity.Jobstep, and
Entity.HoldCostRate) and all user-defined attributes. Applies
only when Type is Split Existing Batch.
Attribute Name Name of representative entity attribute(s) that are assigned to
original entities of the group. Applies only when Member
Attributes is Take Specific Representative Values.
 */
class Separate : public ModelComponent {
public: // constructors
	Separate(Model* model, std::string name = "");
	virtual ~Separate() = default;
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


#endif /* SEPARATE_H */

