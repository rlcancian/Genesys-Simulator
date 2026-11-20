/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   Resource.h
 * Author: rafael.luiz.cancian
 *
 * Created on 21 de Agosto de 2018, 16:52
 */

#ifndef RESOURCE_H
#define RESOURCE_H

#include "../../../kernel/simulator/model/ModelDataDefinition.h"
#include "../../../kernel/simulator/essentialPlugins/StatisticsCollector.h"
#include "../../../kernel/simulator/model/ModelDataManager.h"
#include "../../../kernel/simulator/essentialPlugins/Counter.h"
#include "kernel/simulator/Plugin.h"
#include "kernel/simulator/OnEventManager.h"

#include "plugins/data/DiscreteProcessing/Failure.h"
#include "plugins/data/DiscreteProcessing/Schedule.h"

#include <functional>



class SeizableItem;

/*!
 * \brief Data definition representing a seizable capacity pool (equipment,
 * staff, etc.), its costing, its optional capacity schedule, and its
 * associated Failure definitions.
 *
 * Arena correspondence: the "Resource module" (Rockwell Automation,
 * *Getting Started with Arena*, "The Basic Process Panel", pp. 46-48).
 * \c _capacity is Arena's fixed "Capacity"; \c _capacitySchedule (a
 * Schedule*) is Arena's "Based on Schedule" type; \c _costBusyTimeUnit,
 * \c _costIdleTimeUnit and \c _costPerUse are Arena's Busy/Idle-per-time-unit
 * and Per Use costs (GenESyS generalizes Arena's "per hour" to the model's
 * base time unit); \c _failures is Arena's Failures list (see Failure).
 *
 * \c ResourceState (IDLE/BUSY/FAILED/INACTIVE/OTHER) is the closest
 * correspondence to Arena's resource states, but it is a fixed enumeration.
 *
 * Known differences from Arena, to validate against \c Resource.cpp and
 * \c Schedule.cpp before treating them as closed:
 * - Arena's "StateSet" data module lets a modeler define arbitrary named
 *   states mapped onto autostates or failures; this class only exposes the
 *   fixed \c ResourceState enumeration, so a fully custom named-state
 *   catalog has no GenESyS equivalent;
 * - Arena's per-resource "Schedule Rule" (how a capacity decrease behaves
 *   against a busy unit: ignore/preempt/wait) has no field on this class;
 *   the closest candidate is the per-item \c SchedulableItem::Rule on
 *   Schedule, which is a different granularity and needs confirmation.
 */
class Resource : public ModelDataDefinition {
public:
	typedef std::function<void(Resource*) > ResourceEventHandler;
	typedef std::pair<std::pair<ResourceEventHandler, ModelComponent*>, unsigned int> SortedResourceEventHandler;

	template<typename Class>
	static ResourceEventHandler SetResourceEventHandler(void (Class::*function)(Resource*), Class * object) {
		return std::bind(function, object, std::placeholders::_1);
	}

	enum class ResourceState : int {
		IDLE = 0, BUSY = 1, FAILED = 2, INACTIVE = 3, OTHER = 4, num_elements = 5
	};
public:	
	static std::string convertEnumToStr(ResourceState state);
public:
	//Resource(Model* model);
	Resource(Model* model, std::string name = "");
	virtual ~Resource() override;
public:
	virtual std::string show() override;
public: // static
	static PluginInformation* GetPluginInformation();
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
public:
	bool seize(unsigned int quantity, double priority = 0);
	void release(unsigned int quantity);
	void insertFailure(Failure* failure);
	void removeFailure(Failure* failure);
	double getInstantCapacityUtilization() const;
	double getCapacityUtilization() const;
	double getSeizedUtilization() const;
	double getLastTimeSeized() const; // used only by "Release" component
	void addReleaseResourceEventHandler(ResourceEventHandler eventHandler, ModelComponent* component, unsigned int priority);
public: // g&s
	void setResourceState(ResourceState _resourceState);
	Resource::ResourceState getResourceState() const;
	void setCapacity(unsigned int capacity);
	unsigned int getCapacity() const;
	void setCostBusyTimeUnit(double _costBusyTimeUnit);
	double getCostBusyTimeUnit() const;
	void setCostIdleTimeUnit(double _costIdleTimeUnit);
	double getCostIdleTimeUnit() const;
	void setCostPerUse(double _costPerUse);
	double getCostPerUse() const;
	void setCapacitySchedule(Schedule* _capacitySchedule);
	Schedule* getCapacitySchedule() const;
	unsigned int getNumberBusy() const;

protected: // protected must override
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
protected: // protected could override
	virtual bool _check(std::string& errorMessage) override;
	// virtual void _createInternalAndAttachedData() override;
	virtual void _initBetweenReplications() override;


protected:
	// virtual void _createInternalStatisticReporters() override;
	// virtual void _createNonEditableDataDefinitions() override;
	// virtual void _createEditableDataDefinitions() override;
	virtual void _createAttachedAttributes() override;

private: //methods
	void _notifyReleaseEventHandlers(); //!< Notify observer classes that some of the resource capacity has been released. It is useful for allocation components (such as Seize) to know when an entity waiting into a queue can try to seize the resource again
	void _onReplicationEnd(SimulationEvent* se); //!< Nofified whe replication ended to update cstats based on final replication length
	void _fail();
	void _active();
	void _checkFailByCount();
	void _initStatisticsAndAccounting();
	friend class Failure;
	friend class ResourceTestProbe;

private:

	const struct DEFAULT_VALUES {
		const unsigned int capacity = 1;
		const double cost = 1.0;
		const ResourceState resourceState = ResourceState::IDLE;
	} DEFAULT;
	unsigned int _capacity = DEFAULT.capacity;
	double _costBusyTimeUnit = DEFAULT.cost;
	double _costIdleTimeUnit = DEFAULT.cost;
	double _costPerUse = DEFAULT.cost;
	ResourceState _resourceState = DEFAULT.resourceState;
private: // only gets
	unsigned int _numberBusy = 0;
	double _lastTimeSeized = 0.0; // @TODO: It won't work for resources with capacity>1, when not all capacity is seized and them some more are seized. Seized time of first units will be lost. I don't have a solution so far
	double _lastTimeReleased = 0.0;
	double _lastTimeFailed = 0.0;
	double _lastTimeCapacityEvaluated = 0.0;
	double _lastTimeAnythingNumberBusy = 0.0;
	double _lastTimeIdle = 0.0;
	double _lastTimeBusy = 0.0;
	double _sumNumberBusyOverTime = 0.0;
	double _sumCapacityOverTime = 0.0;
	bool _isActive = true;
	bool _replicationEndHandlerRegistered = false;
private: // not gets nor sets
	unsigned int _originalCapacity; // used for failing purposes, when _capacity changes to 0
private: //1::n
	List<SortedResourceEventHandler*>* _resourceEventHandlers = new List<SortedResourceEventHandler*>();
	List<Failure*>* _failures = new List<Failure*>();
private: // attached elements
	Schedule* _capacitySchedule = nullptr;
private: // internal elements
	StatisticsCollector* _cstatTimeSeized = nullptr;
	StatisticsCollector* _cstatTimeFailed = nullptr;
	StatisticsCollector* _cstatProportionSeized = nullptr;
	StatisticsCollector* _cstatCapacityUtilization = nullptr;
	Counter* _counterTotalTimeSeized = nullptr;
	Counter* _counterTotalTimeFailed = nullptr;
	Counter* _counterNumSeizes = nullptr;
	Counter* _counterNumReleases = nullptr;
	Counter* _counterTotalCostPerUse = nullptr;
	Counter* _counterTotalCostBusy = nullptr;
	Counter* _counterTotalCostIdle = nullptr;
};

#endif /* RESOURCE_H */
