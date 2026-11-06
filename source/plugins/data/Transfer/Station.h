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

#include "../../../kernel/simulator/model/ModelDataDefinition.h"
#include "../../../kernel/simulator/model/ModelDataManager.h"
#include "kernel/simulator/Plugin.h"
#include "../../../kernel/simulator/essentialPlugins/Entity.h"

/*!
 * \brief Data definition representing one physical/logical processing
 * location, tracking the entities currently in it and their sojourn time.
 *
 * Arena correspondence: Arena's "Station module" (Rockwell Automation,
 * *Getting Started with Arena*, "The Advanced Transfer Panel", pp. 86-87).
 * Note that in Arena, Station is a flowchart module placed on the diagram
 * (with a matching Activity Area for cost/time roll-up), whereas in
 * GenESyS this class is purely a data definition referenced by
 * MaterialHandling components (Enter, Route, PickStation, etc.); \c enter()/
 * \c leave() and the \c NumberInStation/TimeInStation statistics collectors
 * are the closest correspondence to Arena's per-station/Activity-Area
 * counts and times.
 *
 * Known differences from Arena, still to verify against the MaterialHandling
 * components in Phase B: no station-set concept (Arena's "Station Type =
 * Set" with member list and Save Attribute), no parent Activity Area for
 * hierarchical cost roll-up, and no associated-intersection field for a
 * guided transporter network — GenESyS currently has no Transporter/Network
 * data definitions at all (see the compatibility matrix).
 */
class Station : public ModelDataDefinition {
public:
	Station(Model* model, std::string name = "");
	virtual ~Station();
public:
	virtual std::string show() override;
public: // static 
	static PluginInformation* GetPluginInformation();
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
public:
	void initBetweenReplications();
	void enter(Entity* entity);
	void leave(Entity* entity);
	void setEnterIntoStationComponent(ModelComponent* _enterIntoStationComponent);
	ModelComponent* getEnterIntoStationComponent() const;
protected:
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
	virtual bool _check(std::string& errorMessage) override;
	// virtual void _createInternalAndAttachedData() override;
	virtual void _initBetweenReplications() override;

protected:
	virtual void _createInternalStatisticReporters() override;
	// virtual void _createNonEditableDataDefinitions() override;
	// virtual void _createEditableDataDefinitions() override;
	virtual void _createAttachedAttributes() override;

private:
	void _initCStats();
	unsigned int _numberInStation = 0;
	ModelComponent* _enterIntoStationComponent = nullptr;
private: // inner elements
	StatisticsCollector* _cstatNumberInStation = nullptr;
	StatisticsCollector* _cstatTimeInStation = nullptr;
	friend class StationTestProbe;
};

#endif /* STATION_H */