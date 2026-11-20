/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   Seize.h
 * Author: rafael.luiz.cancian
 *
 * Created on 21 de Agosto de 2018, 16:17
 */

#ifndef SEIZE_H
#define SEIZE_H

#include <string>
#include "../../../kernel/simulator/model/ModelComponent.h"
#include "../../../kernel/simulator/model/Model.h"
#include "plugins/data/DiscreteProcessing/Resource.h"
#include "plugins/data/DiscreteProcessing/Queue.h"
#include "kernel/simulator/Plugin.h"
#include "auxiliar/SeizableItem.h"
#include "auxiliar/QueueableItem.h"

class WaitingResource : public Waiting {
public:

	WaitingResource(Entity* entity, double timeStartedWaiting, unsigned int quantity, ModelComponent* thisComponent, unsigned int thisComponentOutputPort = 0) : Waiting(entity, timeStartedWaiting, thisComponent, thisComponentOutputPort) {
		_quantity = quantity;
	}

	WaitingResource(const WaitingResource& orig) : Waiting(orig) {
	}

	virtual ~WaitingResource() = default;
public:

	virtual std::string show() override {
		return Waiting::show() +
				",quantity=" + std::to_string(this->_quantity);
	}
public:

	unsigned int getQuantity() const {
		return _quantity;
	}

protected:
private:
	unsigned int _quantity;
};

/*!
 * \brief Allocates one or more resources (or resource-set members) to an
 * entity, queueing it if they are not simultaneously available.
 *
 * Arena correspondence: the "Seize module" (Rockwell Automation, *Getting
 * Started with Arena*, "The Advanced Process Panel", pp. 62-63).
 * `_seizeRequests` (a list of `SeizableItem`) matches Arena's Type/Resource
 * Name/Set Name/Quantity/Selection Rule/Save Attribute fields;
 * `_queueableItem` matches Queue Type; `_priority`/`_priorityExpression`
 * match Priority; `_allocationType` matches Allocation.
 *
 * Confirmed behavior: on a successful seize, stamps
 * `Entity.Allocation.<ResourceName>` with `_allocationType` — this is read
 * back by Release, which credits the resource's held time to that category
 * (see `docs/ai_assistants/reference/ARENA_GENESYS_COMPATIBILITY.md` §6.10
 * and §6.3 for the full category-time-allocation mechanism shared with
 * Delay).
 *
 * Known difference from Arena: no "Resource State" field (post-seize state
 * assignment) — GenESyS's `ResourceState` is a fixed enum, not a
 * user-defined StateSet (see Resource, §5.4).
 */
class Seize : public ModelComponent {
public:
	Seize(Model* model, std::string name = "");
	virtual ~Seize() override;
public:
	virtual std::string show() override;
public:
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
public: // get & set
	void setPriority(unsigned short _priority);
	unsigned short getPriority() const;
	void setAllocationType(Util::AllocationType _allocationType);
	Util::AllocationType getAllocationType() const;
	// indirect access to Queue* and Resource*
	//void setResourceName(std::string _resourceName) throw ();
	//std::string getResourceName() const;
	//void setQueueName(std::string queueName) throw ();
	//std::string getQueueName() const;
	void setQueue(Queue* queue); //!< Deprected
	//Queue* getQueue() const;
	List<SeizableItem*>* getSeizeRequests() const;
    void addRequest(SeizableItem* newRequest);
    void removeRequest(SeizableItem* request);
	void setQueueableItem(QueueableItem* _queueableItem);
	QueueableItem* getQueueableItem() const;
	void setPriorityExpression(std::string _priorityExpression);
	std::string getPriorityExpression() const;
protected:
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override;
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _initBetweenReplications() override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
	virtual bool _check(std::string& errorMessage) override;
protected:
	// virtual void _createInternalStatisticReporters() override;
	// virtual void _createNonEditableDataDefinitions() override;
	virtual void _createEditableDataDefinitions() override;
	// virtual void _createAttachedAttributes() override;
	// virtual void _createInternalAndAttachedData() override;
private:
	void _handlerForResourceEvent(Resource* resource); //!< This method is indirectally invocked (notified) by resource when it's released, since it was added as ResourceEventHandler
	Resource* _getResourceFromSeizableItem(SeizableItem* seizable, Entity* entity, unsigned int*indexPtr);
	Queue* _getQueue() const;
public:

	const struct DEFAULT_VALUES {
		const Util::AllocationType allocationType = Util::AllocationType::Others;
		const unsigned short priority = 0;
		const std::string priorityExpression = "";
		const unsigned int seizeRequestSize = 1;
	} DEFAULT;
private:
	Util::AllocationType _allocationType = DEFAULT.allocationType; // uint ? enum?
	unsigned short _priority = DEFAULT.priority;
	std::string _priorityExpression = DEFAULT.priorityExpression;
	QueueableItem* _queueableItem = nullptr; // usually has a queue, but not always (it could be a hold or a set)
	List<SeizableItem*>* _seizeRequests = new List<SeizableItem*>();
};

#endif /* SEIZE_H */
