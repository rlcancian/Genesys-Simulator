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

#include "../../../kernel/simulator/model/ModelComponent.h"
#include "plugins/data/DiscreteProcessing/Queue.h"

/*!
 * \brief Brings together one entity from each of several input queues,
 * optionally requiring a matching attribute value, and releases them in
 * sync.
 *
 * Arena correspondence: the "Match module" (Rockwell Automation, *Getting
 * Started with Arena*, "The Advanced Process Panel", p. 56). `Rule`
 * (`Any/ByAttribute`) and `_attributeName` match Arena's Type/Attribute
 * Name; `_matchSize` matches Number to Match; `_numberOfQueues` generalizes
 * Arena's fixed five-queue limit to an arbitrary count.
 */
class Match : public ModelComponent {
public:

	enum class Rule : int {
		Any = 0, ByAttribute = 1, num_elements = 2
	};
public:
	static std::string convertEnumToStr(Rule rule);
public: // constructors
	Match(Model* model, std::string name = "");
	virtual ~Match() override;
public: // virtual
	virtual std::string show() override;
public: // static
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
public: // virtual
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override;
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
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
	// virtual void _createInternalStatisticReporters() override;
	// virtual void _createNonEditableDataDefinitions() override;
	virtual void _createEditableDataDefinitions() override;
	// virtual void _createAttachedAttributes() override;
	virtual bool _check(std::string& errorMessage) override;
	// virtual void _createInternalAndAttachedData() override;

protected:

private: // methods
private: // attributes 1:1

	const struct DEFAULT_VALUES {
		const Match::Rule rule = Match::Rule::Any;
		const unsigned int numberOfQueues = 2;
		const std::string matchSize = "1";
		const std::string attributeName = "";
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
