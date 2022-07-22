/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Match.h
 * Author: rlcancian
 *
 * Created on 03 de Junho de 2019, 15:14
 */

#ifndef MATCH_H
#define MATCH_H

#include "../../kernel/simulator/ModelComponent.h"
#include "../data/Queue.h"

/*!
Match module
DESCRIPTION
The Match module brings together a specified number of entities waiting in different
queues. The match may be accomplished when there is at least one entity in each of
the desired queues. Additionally, an attribute may be specified such that the entities
waiting in the queues must have the same attribute values before the match is
initiated.
When an entity arrives at the Match module, it is placed in one of up to five
associated queues, based on the entry point to which it is connected. Entities will
remain in their respective queues until a match exists.
Once a match exists, one entity from each queue is released to be matched. The
matched entities are then synchronized to depart from the module.
TYPICAL USES
* Assembling a part
* Gathering various products for a customer order
* Synchronizing a customer exit with a filled order
Prompt Description
Name Unique module identifier displayed on the module shape.
Number to Match Number of matching entities that must reside in different queues
before a match may be completed.
Type Method for matching the incoming entities. If Type is Any
Entities, one entity must reside in each queue for a match to be
made. If Type is Based on Attribute, one entity must reside in
each queue with the same attribute value.
Attribute Name Attribute name that is used for identifying an arriving entity’s
match value. Applies only when Type is Based on Attribute.
 */
class Match : public ModelComponent {
public:

	enum class Rule : int {
		Any = 0, ByAttribute = 1
	};
public: // constructors
	Match(Model* model, std::string name = "");
	virtual ~Match() = default;
public: // virtual
	virtual std::string show();
public: // static
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
public: // virtual
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber);
	virtual bool _loadInstance(PersistenceRecord *fields);
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues);
public:
	void setRule(Match::Rule _rule);
	Match::Rule getRule() const;
	void setAttributeName(std::string _attributeName);
	std::string getAttributeName() const;
	void setMatchSize(std::string _matchSize);
	std::string getMatchSize() const;
	void setNumberOfQueues(unsigned int _numberOfQueues);
	unsigned int getNumberOfQueues() const;
protected: // virtual
	//virtual void _initBetweenReplications();
	virtual bool _check(std::string* errorMessage);
	virtual void _createInternalAndAttachedData();
private: // methods
private: // attributes 1:1

	const struct DEFAULT_VALUES {
		Match::Rule rule = Match::Rule::Any;
		unsigned int numberOfQueues = 2;
		std::string matchSize = "1";
		std::string attributeName = "";
	} DEFAULT;
	Match::Rule _rule = DEFAULT.rule;
	unsigned int _numberOfQueues = DEFAULT.numberOfQueues;
	std::string _matchSize = DEFAULT.matchSize;
	std::string _attributeName = DEFAULT.attributeName;
private: // attributes 1:1
	std::map<Queue*, std::map<double, unsigned int>*>* _entitiesByAttrib = new std::map<Queue*, std::map<double, unsigned int>*>();
private: // attributes 1:n
	List<Queue*>* _queues = new List<Queue*>();
};


#endif /* MATCH_H */

