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

#include "ModelElement.h"
#include "StatisticsCollector.h"
#include "ElementManager.h"
#include "Counter.h"
#include "Plugin.h"

class Resource;

class ResourceItemRequest {
public:

	ResourceItemRequest(Resource* resource, std::string quantityExpression = "1") {
		_resource = resource;
		_quantityExpression = quantityExpression;
	}

	std::string quantityExpression() const {
		return _quantityExpression;
	}

	Resource* resource() const {
		return _resource;
	}

    void setQuantityExpression(std::string _quantityExpression) {
    	this->_quantityExpression = _quantityExpression;
    }

    void setResource(Resource* _resource) {
    	this->_resource = _resource;
    }
private:
	Resource* _resource;
	std::string _quantityExpression;
};

/*!
Resource module
DESCRIPTION
This data module defines the resources in the simulation system, including costing
information and resource availability. Resources may have a fixed capacity that does
not vary over the simulation run or may operate based on a schedule. Resource
failures and states can also be specified in this module.
TYPICAL USES
 Equipment (machinery, cash register, phone line)
 People (clerical, order processing, sales clerks, operators)
PROMPTS
Prompt Description
Name The name of the resource whose characteristics are being
defined. This name must be unique.
Type Method for determining the capacity for a resource. Fixed
Capacity will not change during the simulation run. Based on
Schedule signifies that a Schedule module is used to specify the
capacity and duration information for the resource.
Capacity Number of resource units of a given name that are available to
the system for processing. Applies only when Type is Fixed
Capacity.
Schedule Name Identifies the name of the schedule to be used by the resource.
The schedule defines the capacity of a resource for a given
period of time. Applies only when type is Schedule.
Schedule Rule Dictates when the actual capacity change is to occur when a
decrease in capacity is required for a busy resource unit. Applies
only when Type is Schedule.
Busy/Hour Cost per hour of a resource that is processing an entity. The
resource becomes busy when it is originally allocated to an entity
and becomes idle when it is released. During the time when it is
busy, cost will accumulate based on the busy/hour cost. The busy
cost per hour is automatically converted to the appropriate base
time unit specified within the Replication Parameters page of the
Run > Setup menu item.
Idle/Hour Cost per hour of a resource that is idle. The resource is idle while
it is not processing an entity. During the time when it is idle, cost
will accumulate based on the idle/hour cost. The idle cost per
hour is automatically converted to the appropriate base time unit
specified within the Replication Parameters page of the Run >
Setup menu item.
Per Use Cost of a resource on a usage basis, regardless of the time for
which it is used. Each time the resource is allocated to an entity,
it will incur a per-use cost.
StateSet Name Name of states that the resource may be assigned during the
simulation run.
Initial State Initial state of a resource. If specified, the name must be defined
within the repeat group of state names. This field is shown only
when a StateSet Name is defined.
Failures Lists all failures that will be associated with the resource.
 Failure Name–Name of the failure associated with the
resource.
 Failure Rule–Behavior that should occur when a failure is to
occur for a busy resource unit.
Report Statistics Specifies whether or not statistics will be collected automatically
and stored in the report database for this resource.
 */
class Resource : public ModelElement {
public:
	typedef std::function<void(Resource*) > ResourceEventHandler;

	template<typename Class>
	static ResourceEventHandler SetResourceEventHandler(void (Class::*function)(Resource*), Class * object) {
		return std::bind(function, object, std::placeholders::_1);
	}

	enum class ResourceType : int {
		SET = 1, RESOURCE = 2
	};

	enum class ResourceRule : int {
		RANDOM = 1, CICLICAL = 2, ESPECIFIC = 3, SMALLESTBUSY = 4, LARGESTREMAININGCAPACITY = 5
	};

	enum class ResourceState : int {
		IDLE = 1, BUSY = 2, FAILED = 3, INACTIVE = 4, OTHER = 5
	};

public:
	//Resource(Model* model);
	Resource(Model* model, std::string name = "");
	virtual ~Resource();
public:
	virtual std::string show();
public:
	static PluginInformation* GetPluginInformation();
	static ModelElement* LoadInstance(Model* model, std::map<std::string, std::string>* fields);
public:
	void seize(unsigned int quantity, double tnow);
	void release(unsigned int quantity, double tnow);
	void initBetweenReplications();
public: // g&s
	void setResourceState(ResourceState _resourceState);
	Resource::ResourceState getResourceState() const;
	void setCapacity(unsigned int _capacity);
	unsigned int getCapacity() const;
	void setCostBusyHour(double _costBusyHour);
	double getCostBusyHour() const;
	void setCostIdleHour(double _costIdleHour);
	double getCostIdleHour() const;
	void setCostPerUse(double _costPerUse);
	double getCostPerUse() const;
public: // gets
	unsigned int getNumberBusy() const;
public:
	void addReleaseResourceEventHandler(ResourceEventHandler eventHandler);
	double getLastTimeSeized() const;
protected:
	virtual bool _loadInstance(std::map<std::string, std::string>* fields);
	virtual std::map<std::string, std::string>* _saveInstance();
	virtual bool _check(std::string* errorMessage);
	virtual void _createInternalElements();

private:
	void _notifyReleaseEventHandlers(); ///< Notify observer classes that some of the resource capacity has been released. It is useful for allocation components (such as Seize) to know when an entity waiting into a queue can try to seize the resource again
	//private:
	//    ElementManager* _elems;
private:
	unsigned int _capacity = 1;
	double _costBusyHour = 1.0;
	double _costIdleHour = 1.0;
	double _costPerUse = 1.0;
	ResourceState _resourceState = ResourceState::IDLE;
private: // only gets
	unsigned int _numberBusy = 0;
	//unsigned int _numberOut = 0;
	double _lastTimeSeized = 0.0; // \todo: It won't work for resources with capacity>1, when not all capacity is seized and them some more are seized. Seized time of first units will be lost. I don't have a solution so far
private: // not gets nor sets
	//unsigned int _seizes = 0;
	//double _whenSeized; // same as last? check
private: //1::n
	List<ResourceEventHandler>* _resourceEventHandlers = new List<ResourceEventHandler>();
	//aFailures:	TStringList;
	//std::list<Failure*>* _failures;
private: // inner children elements
	StatisticsCollector* _cstatTimeSeized = nullptr;
	Counter* _numSeizes;
	Counter* _numReleases;
};

#endif /* RESOURCE_H */

