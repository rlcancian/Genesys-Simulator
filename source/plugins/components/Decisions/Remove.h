/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Remove.h
 * Author: rlcancian
 *
 * Created on 03 de Junho de 2019, 15:20
 */

#ifndef REMOVE_H
#define REMOVE_H

#include "kernel/simulator/ModelComponent.h"

/*!
Remove module
DESCRIPTION
The Remove module removes a single entity from a specified position in a queue and
sends it to a designated module.
When an entity arrives at a Remove module, it removes the entity from the specified
queue and sends it to the connected module. The rank of the entity signifies the
location of the entity within the queue. The entity that caused the removal proceeds to
the next module specified and is processed before the removed entity.
TYPICAL USES
 * Removing an order from a queue that is due to be completed next
 * Calling a patient from a waiting room for an examination
 * Retrieving the next order to be processed from a pile of documents
Prompt Description
Name Unique module identifier displayed on the module shape.
Queue Name Name of the queue from which the entity will be removed.
Rank of Entity Rank of the entity to remove from within the queue.
 */
class Remove : public ModelComponent {
public:

	enum class RemoveFromType : int {
		QUEUE = 0, ENTITYGROUP = 1, num_elements = 2
	};
public:
	static std::string convertEnumToStr(RemoveFromType type);
public: // constructors
	Remove(Model* model, std::string name = "");
	virtual ~Remove() = default;
public: // virtual
	virtual std::string show() override;
public:
	void setRemoveStartRank(std::string _removeFromRank);
	std::string getRemoveStartRank() const;
    void setRemoveEndRank(std::string _removeEndRank);
    std::string getRemoveEndRank() const;	
	void setRemoveFromType(Remove::RemoveFromType _removeFromType);
	Remove::RemoveFromType getRemoveFromType() const;
	void setRemoveFrom(ModelDataDefinition* _removeFrom);
	ModelDataDefinition* getRemoveFrom() const;

public: // static
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
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
	virtual void _createInternalAndAttachedData() override; /*< A ModelDataDefinition or ModelComponent that includes (internal) ou refers to (attach) other ModelDataDefinition must register them inside this method. */
	//virtual void _addProperty(PropertyBase* property);
private: // methods
private: // attributes 1:1

	const struct DEFAULT_VALUES {
		const Remove::RemoveFromType removeFromType = Remove::RemoveFromType::QUEUE;
		const std::string removeStartRank = "";
		const std::string removeEndRank = "";
	} DEFAULT;
	ModelDataDefinition* _removeFrom = nullptr;
	Remove::RemoveFromType _removeFromType = DEFAULT.removeFromType;
	std::string _removeStartRank = DEFAULT.removeStartRank;
	std::string _removeEndRank = DEFAULT.removeStartRank;
	//std::string _removeFromRank = DEFAULT.removeFromRank;

private: // attributes 1:n
};


#endif /* REMOVE_H */
