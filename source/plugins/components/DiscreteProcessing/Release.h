/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Release.h
 * Author: rafael.luiz.cancian
 *
 * Created on 21 de Agosto de 2018, 16:17
 */

#ifndef RELEASE_H
#define RELEASE_H

#include <string>

#include "../../../kernel/simulator/model/ModelComponent.h"
#include "kernel/simulator/Plugin.h"
#include "auxiliar/SeizableItem.h"
#include "plugins/data/DiscreteProcessing/Resource.h"

/*!
 * \brief Gives up control of one or more previously seized resources,
 * immediately unblocking any entity waiting for them.
 *
 * Arena correspondence: the "Release module" (Rockwell Automation, *Getting
 * Started with Arena*, "The Advanced Process Panel", p. 60).
 * `_releaseRequests` (a list of `SeizableItem`, reused from Seize) matches
 * Arena's Type/Resource Name/Set Name/Quantity/Release Rule fields.
 *
 * Confirmed behavior: reads back `Entity.Allocation.<ResourceName>` (set by
 * Seize) and credits the resource's held time
 * (`resource->getLastTimeSeized()`) to that category, both into a
 * `<EntityType>.<Category>Time` StatisticsCollector and a running
 * `Entity.Total<Category>Time` attribute — see
 * `docs/ai_assistants/reference/ARENA_GENESYS_COMPATIBILITY.md` §6.10.
 */
class Release : public ModelComponent {
public:
	Release(Model* model, std::string name = "");
	virtual ~Release() override;
public:
	virtual std::string show() override;
public: //static
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
public: // get & set
	void setPriority(unsigned short _priority);
	unsigned short priority() const;
public: // gets
	List<SeizableItem*>* getReleaseRequests() const;
	void addReleaseRequests(SeizableItem* newRequest);
	void removeReleaseRequests(SeizableItem* request);

protected:
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override;
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _initBetweenReplications() override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
	virtual bool _check(std::string& errorMessage) override;
	// virtual void _createInternalAndAttachedData() override;

protected:
	// virtual void _createInternalStatisticReporters() override;
	// virtual void _createNonEditableDataDefinitions() override;
	virtual void _createEditableDataDefinitions() override;
	// virtual void _createAttachedAttributes() override;

private:
	Resource* _getResourceFromSeizableItem(SeizableItem* seizable, Entity* entity);
private:

	const struct DEFAULT_VALUES {
		const unsigned short priority = 0;
		const unsigned int releaseRequestSize = 1;
	} DEFAULT;
	unsigned short _priority = DEFAULT.priority;
	List<SeizableItem*>* _releaseRequests = new List<SeizableItem*>();
};

#endif /* RELEASE_H */
