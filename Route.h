/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Route.h
 * Author: rlcancian
 *
 * Created on 03 de Junho de 2019, 15:15
 */

#ifndef ROUTE_H
#define ROUTE_H

#include "ModelComponent.h"
#include "Station.h"
#include "Counter.h"

/*!
Route module
DESCRIPTION
The Route module transfers an entity to a specified station or the next station in the
station visitation sequence defined for the entity. A delay time to transfer to the next
station may be defined. 
When an entity enters the Route module, its Station attribute (Entity.Station) is set to
the destination station. The entity is then sent to the destination station, using the
route time specified.
If the station destination is entered as By Sequence, the next station is determined by
the entity’s sequence and step within the set (defined by special-purpose attributes
Entity.Sequence and Entity.Jobstep, respectively).
TYPICAL USES
 Send a part to its next processing station based on its routing slip
 Send an account balance call to an account agent
 Send restaurant customers to a specific table
PROMPTS
Prompt Description
Name Unique name of the module that will be displayed in the
flowchart.
Route Time Travel time from the entity’s current location to the destination
station.
Units Time units for route-time parameters.
Destination Type Method for determining the entity destination location. Selection
of By Sequence requires that the entity has been assigned a
sequence name and that the sequence itself has been defined.
Station Name Name of the individual destination station.
Attribute Name Name of the attribute that stores the station name to which
entities will route.
Expression Expression that is evaluated to the station name where entities
will route.
 */
class Route : public ModelComponent {
public:

	enum class DestinationType : int {
		Station = 0, BySequence = 1
	};
public:
	Route(Model* model, std::string name = "");
	virtual ~Route() = default;
public:
	virtual std::string show();
public:
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, std::map<std::string, std::string>* fields);
public:
	void setStation(Station* _station);
	Station* getStation() const;
	void setRouteTimeExpression(std::string _routeTimeExpression);
	std::string getRouteTimeExpression() const;
	void setRouteTimeTimeUnit(Util::TimeUnit _routeTimeTimeUnit);
	Util::TimeUnit getRouteTimeTimeUnit() const;
	void setRouteDestinationType(DestinationType _routeDestinationType);
	Route::DestinationType getRouteDestinationType() const;
public:
protected:
	virtual void _execute(Entity* entity);
	virtual void _initBetweenReplications();
	virtual bool _loadInstance(std::map<std::string, std::string>* fields);
	virtual std::map<std::string, std::string>* _saveInstance();
	virtual bool _check(std::string* errorMessage);
	virtual void _createInternalElements();
private:
	std::string _routeTimeExpression = "0.0";
	Util::TimeUnit _routeTimeTimeUnit = Util::TimeUnit::second;
	Route::DestinationType _routeDestinationType = DestinationType::Station;
private: // association
	Station* _station;
private: // children elements
	Counter* _numberIn = nullptr;
};

#endif /* ROUTE_H */

