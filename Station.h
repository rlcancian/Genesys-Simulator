/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Station.h
 * Author: rlcancian
 *
 * Created on 03 de Junho de 2019, 15:12
 */

#ifndef STATION_H
#define STATION_H

#include "ModelElement.h"
#include "ElementManager.h"
#include "Plugin.h"
#include "Entity.h"

/*
 Station module
DESCRIPTION
The Station module defines a station (or a set of stations) corresponding to a physical
or logical location where processing occurs. If the Station module defines a station
set, it is effectively defining multiple processing locations.
The station (or each station within the defined set) has a matching Activity Area that
is used to report all times and costs accrued by the entities in this station. This
Activity Area’s name is the same as the station. If a parent Activity Area is defined,
then it also accrues any times and costs by the entities in this station.
TYPICAL USES
 Defining a lathe area
 Defining a set of toll booths
 Defining a food preparation area
PROMPTS
Prompt Description
Name Unique name of the module that will be displayed in the
flowchart.
Station Type Type of station being defined, either as an individual Station or a
station Set.
Station Name Name of the individual station.
Set Name Name of the station set.
Parent Activity Area Name of the Activity Area’s parent.
Associated Intersection Name of the intersection associated with this station in a guided
transporter network.
Report Statistics Specifies whether or not statistics will automatically be collected
and stored in the report database for this station and its
corresponding activity area.
Save Attribute Attribute name used to store the index number into the station set
of the member that is selected.
Station Set Members Names of the stations that are members of this station set.
Station Name A given station can only exist once within a model. Therefore, an
individual station can only be the member of one station set, and
that individual station may not be the name of a station in
another module.
Parent Activity Area Name of the Activity Area’s parent for the station set member.
Associated Intersection Name of the intersection associated with this station set in a
guided transporter network.
Report Statistics Specifies whether or not statistics will automatically be collected
and stored in the report database for this station set member and
its corresponding activity area. 
 */
class Station : public ModelElement {
public:
	Station(Model* model, std::string name = "");
	virtual ~Station();
public:
	virtual std::string show();
public: // static 
	static PluginInformation* GetPluginInformation();
	static ModelElement* LoadInstance(Model* model, std::map<std::string, std::string>* fields);
public:
	void initBetweenReplications();
	void enter(Entity* entity);
	void leave(Entity* entity);
	void setEnterIntoStationComponent(ModelComponent* _enterIntoStationComponent);
	ModelComponent* getEnterIntoStationComponent() const;
protected:
	virtual bool _loadInstance(std::map<std::string, std::string>* fields);
	virtual std::map<std::string, std::string>* _saveInstance();
	virtual bool _check(std::string* errorMessage);
	virtual void _createInternalElements();
private:
	unsigned int _numberInStation = 0;
	ModelComponent* _enterIntoStationComponent;
private: // inner elements
	StatisticsCollector* _cstatNumberInStation = nullptr;
	StatisticsCollector* _cstatTimeInStation;
};

#endif /* STATION_H */

