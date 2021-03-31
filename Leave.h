/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Leave.h
 * Author: rlcancian
 *
 * Created on 03 de Junho de 2019, 15:14
 */

#ifndef LEAVE_H
#define LEAVE_H

#include "ModelComponent.h"
#include "Station.h"
#include "Counter.h"

/*!
Leave module
DESCRIPTION
The Leave module is used to transfer an entity to a station or module. An entity may
be transferred in two ways. It can be transferred to a module that defines a station by 
referencing the station and routing, conveying, or transporting to that station, or a
graphical connection can be used to transfer an entity to another module. When an
entity arrives at a Leave module, it may wait to obtain a transfer device (resource,
transporter, or conveyor). When the transfer device has been obtained, the entity may
experience a loading delay. Finally, the entity is transferred from this module to a
destination module or station.
TYPICAL USES
 The end of a part’s production in a series of parallel processes where the part
needs a forklift to be transferred to shipping
PROMPTS
Prompt Description
Name Unique name of the module that will be displayed in the
flowchart.
Allocation Type of category to which the entity’s incurred delay time and
cost will be added.
Delay Specifies a load time incurred after getting a transfer device.
Units Time units used for the delay time.
Transfer Out Determines whether a resource (Seize Resource), transporter
(Request Transporter), or conveyor (Access Conveyor) is
required prior to transferring the entity out of this module.
Priority Indicates the priority of the module when either seizing a
resource or requesting a transporter when there are entities
waiting for that resource/transporter from other modules. This
field is not visible when the Transfer Type is None or Access
Conveyor.
Transporter Name Name of the transporter to request.
Queue Type Type of queue, either a single Queue, queue Set, Internal
queue, Attribute, or Expression.
Queue Name Name of the individual queue.
Queue Set Name Name of the queue set.
Set Index Defines the index into the queue set. Note that this is the index
into the set and not the name of the queue in the set.
Queue Attribute Name The attribute name that will be evaluated to indicate which queue
is to be used.
Queue Expression The expression that will be evaluated to indicate which queue is
to be used.
Selection Rule Method of selecting among available transporters in a set.
Cyclical will cycle through available members. Random will
randomly select a member. Preferred Order will always select
the first available member. Specific Member requires an input
attribute value to specify which member of the set (previously
saved in the Save Attribute field). Largest Distance selects the
transporter farthest away, and Smallest Distance selects the
closest transporter.
Save Attribute Attribute name used to store the index number into the set of the
member that is chosen. This attribute can later be referenced with
the Specific Member selection rule. Active when Transfer Out
is Request Transporter.
Index Set Attribute name whose value identifies the index number into the
set of the member requested. The entity must have a value for the
attribute before utilizing this option.
Resource Type Type of resource for seizing, either specifying a particular
Resource, selecting from a pool of resources (that is, a resource
Set), Attribute, or Expression.
Resource Name Name of the resource to seize.
Conveyor Name Name of the conveyor to access.
# of Cells Number of contiguous cells the entity requires.
Connect Type Determines if the entity is to Route, Convey, or Transport to
another station or Connect to another module.
Move Time Time to route from this module to the destination station.
Units Time units used for the move time.
Station Type The entity’s destination station type either an individual Station,
a station based on an Attribute or Expression value, or By
Sequence.
Station Name Name of the individual destination station.
Attribute Name The attribute name that will be evaluated to indicate the station.
Expression The expression that will be evaluated to indicate the station.
 */
class Leave : public ModelComponent {
public:
	Leave(Model* model, std::string name = "");
	virtual ~Leave() = default;
public:
	virtual std::string show();
public:
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, std::map<std::string, std::string>* fields);
public:
	void setStation(Station* _station);
	Station* getStation() const;
public:
protected:
	virtual void _execute(Entity* entity);
	virtual void _initBetweenReplications();
	virtual bool _loadInstance(std::map<std::string, std::string>* fields);
	virtual std::map<std::string, std::string>* _saveInstance();
	virtual bool _check(std::string* errorMessage);
	virtual void _createInternalElements();
private: // association
	Station* _station;
private: // children elements
	Counter* _numberIn = nullptr;
};

#endif /* LEAVE_H */

