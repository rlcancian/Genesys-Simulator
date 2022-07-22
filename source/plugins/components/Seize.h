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
#include "../../kernel/simulator/ModelComponent.h"
#include "../../kernel/simulator/Model.h"
#include "../data/Resource.h"
#include "../data/Queue.h"
#include "../../kernel/simulator/Plugin.h"
#include "SeizableItem.h"
#include "QueueableItem.h"

class WaitingResource : public Waiting {
public:
	WaitingResource(Entity* entity, double timeStartedWaiting, unsigned int quantity, ModelComponent* thisComponent, unsigned int thisComponentOutputPort = 0) : Waiting(entity, timeStartedWaiting, thisComponent, thisComponentOutputPort) {
		_quantity = quantity;
	}
	WaitingResource(const WaitingResource& orig) : Waiting(orig) {
	}

	virtual ~WaitingResource() = default;
public:
	virtual std::string show() {
		return Waiting::show() +
				",quantity=" + std::to_string(this->_quantity);
	}
public:
	unsigned int getQuantity() const {
		return _quantity;
	}
private:
	unsigned int _quantity;
};

/*!
Seize module
DESCRIPTION
The Seize module allocates units of one or more resources to an entity. The Seize
module may be used to seize units of a particular resource, a member of a resource
set, or a resource as defined by an alternative method, such as an attribute or
expression.
When an entity enters this module, it waits in a queue (if specified) until all specified
resources are available simultaneously. Allocation type for resource usage is also
specified.
TYPICAL USES
* Beginning a customer order (seize the operator)
* Starting a tax return (seize the accountant)
* Being admitted to hospital (seize the hospital room, nurse, doctor)
PROMPTS
Prompt Description
Name Unique module identifier displayed on the module shape.
Allocation Determines to which category the resource usage cost will be
allocated for an entity going through the Seize module.
Priority Priority value of the entity waiting at this module for the
resource(s) specified if one or more entities from other modules
are waiting for the same resource(s).
Type Type of resource for seizing, either specifying a particular
resource, or selecting from a pool of resources (that is, a resource
set). The name of the resource may also be specified as an
attribute value or within an expression.
Resource Name Name of the resource that will be seized.
Set Name Name of the resource set from which a member will be seized.
Attribute Name Name of the attribute that stores the resource name to be seized.
Expression Expression that evaluates to a resource name to be seized.
Quantity Number of resources of a given name or from a given set that
will be seized. For sets, this value specifies only the number of a
selected resource that will be seized (based on the resource’s
capacity), not the number of members to be seized within the set.
Selection Rule Method of selecting among available resources in a set. Cyclical
will cycle through available members (for example, 1-2-3-1-2-
3). Random will randomly select a member. Preferred Order
will always select the first available member (for example, 1, if
available; then 2, if available; then 3). Specific Member requires
an input attribute value to specify which member of the set
(previously saved in the Save Attribute field). Largest
Remaining Capacity and Smallest Number Busy are used for
resources with multiple capacity.
Save Attribute Attribute name used to store the index number into the set of the
member that is chosen. This attribute can later be referenced with
the Specific Member selection rule.
Set Index Index value into the set that identifies the number into the set of
the member requested. If an attribute name is used, the entity
must have a value for the attribute before utilizing this option.
Resource State State of the resource that will be assigned after the resource is
seized. The resource state must be defined with the Resource
module.
Queue Type Determines the type of queue used to hold the entities while
waiting to seize the resource(s). If Queue is selected, the queue
name is specified. If Set is selected, the queue set and member in
the set are specified. If Internal is selected, an internal queue is
used to hold all waiting entities. Attribute and Expression are
additional methods for defining the queue to be used.
Queue Name This field is visible only if Queue Type is Queue, and it defines
the symbol name of the queue.
Set Name This field is visible only if Queue Type is Set, and it defines the
queue set that contains the queue being referenced.
Set Index This field is visible only if Queue Type is Set, and it defines the
index into the queue set. Note that this is the index into the set
and not the name of the queue in the set. For example, the only
valid entries for a queue set containing three members is an
expression that evaluates to 1, 2, or 3.
Attribute This field is visible only if Queue Type is Attribute. The attribute
entered in this field will be evaluated to indicate which queue is
to be used.
Expression This field is visible only if Queue Type is Expression. The
expression entered in this field will be evaluated to indicate
which queue is to be used.
 */
class Seize : public ModelComponent {
public:
	Seize(Model* model, std::string name = "");
	virtual ~Seize() = default;
public:
	virtual std::string show();
public:
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
public: // get & set
	void setPriority(unsigned short _priority);
	unsigned short getPriority() const;
	void setAllocationType(unsigned int _allocationType);
	unsigned int getAllocationType() const;
	// indirect access to Queue* and Resource*
	//void setResourceName(std::string _resourceName) throw ();
	//std::string getResourceName() const;
	//void setQueueName(std::string queueName) throw ();
	//std::string getQueueName() const;
	void setQueue(Queue* queue); ///< Deprected
	//Queue* getQueue() const;
	List<SeizableItem*>* getSeizeRequests() const;
	void setQueueableItem(QueueableItem* _queueableItem);
	QueueableItem* getQueueableItem() const;
	void setSaveAttribute(std::string _saveAttribute);
	std::string getSaveAttribute() const;
protected:
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber);
	virtual bool _loadInstance(PersistenceRecord *fields);
	virtual void _initBetweenReplications();
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues);
	virtual bool _check(std::string* errorMessage);
	virtual void _createInternalAndAttachedData();
private:
	void _handlerForResourceEvent(Resource* resource); ///< This method is indirectally invocked (notified) by resource when it's released, since it was added as ResourceEventHandler
	Resource* _getResourceFromSeizableItem(SeizableItem* seizable, Entity* entity);
	Queue* _getQueue() const;
public:
	const struct DEFAULT_VALUES {
		const unsigned int allocationType = 0; // uint ? enum?
		const unsigned short priority = 0;
		const unsigned int seizeRequestSize = 1;
		const std::string saveAttribute = "";
	} DEFAULT;
private:
	unsigned int _allocationType = DEFAULT.allocationType; // uint ? enum?
	unsigned short _priority = DEFAULT.priority;
	std::string _saveAttribute = DEFAULT.saveAttribute;
	QueueableItem* _queueableItem = nullptr; // usually has a queue, but not always (it could be a hold or a set)
	List<SeizableItem*>* _seizeRequests = new List<SeizableItem*>();
};

#endif /* SEIZE_H */

