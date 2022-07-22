/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Batch.h
 * Author: rlcancian
 *
 * Created on 03 de Junho de 2019, 15:14
 */

#ifndef BATCH_H
#define BATCH_H

#include "../../kernel/simulator/ModelComponent.h"
#include "../../plugins/data/Queue.h"
#include "../../plugins/data/EntityGroup.h"
/*!
Batch module
DESCRIPTION
This module is intended as the grouping mechanism within the simulation model.
Batches can be permanently or temporarily grouped. Temporary batches must later be
split using the Separate module.
Batches may be made with any specified number of entering entities or may be
matched together based on an attribute. Entities arriving at the Batch module are
placed in a queue until the required number of entities has accumulated. Once
accumulated, a new representative entity is created.
TYPICAL USES
* Collect a number of parts before starting processing
* Reassemble previously separated copies of a form
* Bring together a patient and his record before commencing an appointment
PROMPTS
Prompt Description
Name Unique module identifier displayed on the module shape.
Type Method of batching entities together.
Batch Size Number of entities to be batched.
Save Criterion Method for assigning representative entity’s user-defined
attribute values.
Rule Determines how incoming entities will be batched. Any Entity
will take the first “Batch Size” number of entities and put them
together. By Attribute signifies that the values of the specified
attribute must match for entities to be grouped. For example, if
Attribute Name is Color, all entities must have the same Color
value to be grouped; otherwise, they will wait at the module for
additional incoming entities.
Attribute Name Name of the attribute whose value must match the value of the
other incoming entities in order for a group to be made. Applies
only when Rule is By Attribute.
Representative Entity The entity type for the representative entity. 

 */
class Batch : public ModelComponent {
public:

	enum class BatchType : int {
		Temporary = 0, Permanent = 1
	};

	enum class Rule : int {
		Any = 0, ByAttribute = 1
	};

	enum class GroupedAttribs : int {
		FirstEntity = 0, LastEntity = 1, SumAttributes = 2
	};
public: // constructors
	Batch(Model* model, std::string name = "");
	virtual ~Batch() = default;
public: // virtual
	virtual std::string show();
public: // static
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
public:
	void setGroupedEntityType(EntityType* groupedEntityType);
	void setGroupedEntityTypeName(std::string groupedEntityTypeName);
	EntityType* getGroupedEntityType() const;
	void setAttributeName(std::string attributeName);
    std::string getAttributeName() const;
	void setBatchSize(std::string batchSize);
    std::string getBatchSize() const;
    void setRule(Batch::Rule _rule);
    Batch::Rule getRule() const;
    void setGroupedAttributes(Batch::GroupedAttribs _groupedAttributes);
	Batch::GroupedAttribs getGroupedAttributes() const;
protected: // virtual should
	//virtual void _initBetweenReplications();
	virtual void _createInternalAndAttachedData();
	virtual bool _check(std::string* errorMessage);
protected: // virtual must
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber);
	virtual bool _loadInstance(PersistenceRecord *fields);
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues);
private: // methods
private: // attributes 1:1

	const struct DEFAULT_VALUES {
		Batch::BatchType batchType = Batch::BatchType::Temporary;
		Batch::Rule rule = Batch::Rule::Any;
		Batch::GroupedAttribs groupedAttributes = Batch::GroupedAttribs::FirstEntity;
		std::string batchSize = "2";
		std::string attributeName = "";
	} DEFAULT;
	Batch::BatchType _batchType = DEFAULT.batchType;
	Batch::Rule _rule = DEFAULT.rule;
	Batch::GroupedAttribs _groupedAttributes = DEFAULT.groupedAttributes;
	std::string _batchSize = DEFAULT.batchSize;
	std::string _attributeName = DEFAULT.attributeName;
private: // attributes 1:1
	EntityType* _groupedEntityType = nullptr;
	EntityGroup* _entityGroup = nullptr;
	Queue* _queue = nullptr;
private: // attributes 1:n
};


#endif /* BATCH_H */

