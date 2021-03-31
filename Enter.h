/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Enter.h
 * Author: rlcancian
 *
 * Created on 03 de Junho de 2019, 15:14
 */

#ifndef ENTER_H
#define ENTER_H

#include "ModelComponent.h"
#include "Station.h"
#include "Counter.h"
/*!
Enter module
DESCRIPTION
The Enter module defines a station (or a set of stations) corresponding to a physical
or logical location where processing occurs. When an entity arrives at an Enter
module, an unloading delay may occur and any transfer device used to transfer the
entity to the Enter module’s station may be released.
The station (or each station within the defined set) has a matching Activity Area that
is used to report all times and costs accrued by the entities in this station. This
Activity Area’s name is the same as the station. If a parent Activity Area is defined,
then it also accrues any times and costs by the entities in this station.
TYPICAL USES
 The start of a part’s production in a series of parallel processes where the part’s
forklift needs to be released
 The start of a document’s processing after the document has been created where
the mail clerk resource needs to be released
PROMPTS
Prompt Description
Name Unique name of the module that will be displayed in the
flowchart
Station Type Type of station, either a single Station or station Set.
Station Name Name of the individual station. A given station can only exist
once within a model.
Parent Activity Area Name of the Activity Area’s parent.
Associated Intersection Name of the intersection associated with this station in a guided
transporter network.
Report Statistics Specifies whether or not statistics will automatically be collected
and stored in the report database for this station and its
corresponding activity area.
Set Name Name of the station set. A given station set can only exist once
within a model.
Save Attribute Specifies the attribute to be used to store the index into the
station set for an entity entering this module.
Set Members This repeat group permits you to define the individual stations
that are to be members of the specified station set. A station set
must have at least one member station. Active when Station Type
is Set.
Station Name This field indicates the name of a station that is to be a member
of this station set. A given station can only exist within a model
once. Therefore, an individual station can only be the member of
one station set, and that individual station may not be the name
of a station in another module.
Parent Activity Area Name of the Activity Area’s parent for the station set member.
Associated Intersection Name of the intersection associated with this station set in a
guided transporter network.
Report Statistics Specifies whether or not statistics will automatically be collected
and stored in the report database for this station set member and
its corresponding activity area.
Allocation Type of category to which the entity’s incurred delay time and
cost will be added.
Delay This field defines the delay that will be experienced by entities
immediately upon arrival at the station. 
Units Time units used for the delay time.
Transfer In If a resource, transporter, or conveyor was used to transfer the
entity to this station, this can be used to release, free, or exit the
device. If Release Resource is selected, the specified resource is
released. If Free Transporter is selected, the specified
transporter is freed. If Exit Conveyor is selected, the specified
conveyor is exited.
Transporter Name Name of the transporter to be freed upon arrival to the station.
Active when Transfer Name is Free Transporter.
Unit Number Unit number of the transporter if the transporter is multicapacity.
Conveyor Name Name of the conveyor to exit upon arrival to the station.
Resource Type Type of allocation, either single Resource or resource Set.
Resource Name Name of the resource to release. Active when Transfer Name is
Release Resource.
Set Name Name of the resource set from which the resource is to be
released.
Release Rule Determines which member of the set is to be released, either the
Last Member Seized, First Member Seized, or Specific
Member.
Set Index Index into the set that determines which member of the set is to
be released.
Attribute Name Name of the attribute that determines the instance number of the
resource to release.
Expression Expression value that determines the instance number of the
resource to release.  
 */
class Enter : public ModelComponent {
public:
	Enter(Model* model, std::string name = "");
	virtual ~Enter() = default;
public:
	virtual std::string show();
public:
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, std::map<std::string, std::string>* fields);
public:
	void setStation(Station* _station);
	Station* getStation() const;
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

#endif /* ENTER_H */

