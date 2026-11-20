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

#include "../../../kernel/simulator/model/ModelComponent.h"
#include "plugins/data/DiscreteProcessing/Queue.h"
#include "../../data/Grouping/EntityGroup.h"
/*!
 * \brief Accumulates entities in an internal queue until a batch is
 * complete, then dispatches one representative entity for the group.
 *
 * Arena correspondence: the "Batch module" (Rockwell Automation, *Getting
 * Started with Arena*, "The Basic Process Panel", p. 38).
 * \c BatchType (Temporary/Permanent), \c Rule (Any/ByAttribute) +
 * \c _attributeName, and \c GroupedAttribs (FirstEntity/LastEntity/
 * SumAttributes) map directly onto Arena's Type, Rule/Attribute Name, and
 * Save Criterion (First/Last/Sum) fields; \c _batchSize is a parsed
 * expression generalizing Arena's static number; \c _groupedEntityType is
 * the Representative Entity type. A temporary batch is later split back
 * into its original members by Separate.
 */
class Batch : public ModelComponent {
public:

	enum class BatchType : int {
		Temporary = 0, Permanent = 1, num_elements = 2
	};

	enum class Rule : int {
		Any = 0, ByAttribute = 1, num_elements = 2
	};

	enum class GroupedAttribs : int {
		FirstEntity = 0, LastEntity = 1, SumAttributes = 2, num_elements = 3
	};
public:
	static std::string convertEnumToStr(BatchType type);
	static std::string convertEnumToStr(Rule rule);
	static std::string convertEnumToStr(GroupedAttribs attribs);
public: // constructors
	Batch(Model* model, std::string name = "");
	virtual ~Batch() = default;
public: // virtual
	virtual std::string show() override;
public: // static
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
public:
	void setGroupedEntityType(EntityType* groupedEntityType);
	void setGroupedEntityTypeName(std::string groupedEntityTypeName);
	EntityType* getGroupedEntityType() const;
	void setBatchType(Batch::BatchType batchType);
	Batch::BatchType getBatchType() const;
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
	// virtual void _createInternalStatisticReporters() override;
	virtual void _createNonEditableDataDefinitions() override;
	virtual void _createEditableDataDefinitions() override;
	virtual void _createAttachedAttributes() override;
	// virtual void _createInternalAndAttachedData() override;
	virtual bool _check(std::string& errorMessage) override;
protected: // virtual must
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override;
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;

protected:

private: // methods
private: // attributes 1:1

	const struct DEFAULT_VALUES {
		const Batch::BatchType batchType = Batch::BatchType::Temporary;
		const Batch::Rule rule = Batch::Rule::Any;
		const Batch::GroupedAttribs groupedAttributes = Batch::GroupedAttribs::FirstEntity;
		const std::string batchSize = "2";
		const std::string attributeName = "";
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
	// count number of batches?
};


#endif /* BATCH_H */
