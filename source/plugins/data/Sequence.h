/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Sequence.h
 * Author: rlcancian
 *
 * Created on 03 de Junho de 2019, 15:12
 */

#ifndef SEQUENCE_H
#define SEQUENCE_H

#include "../../kernel/simulator/ModelDataDefinition.h"
#include "../../kernel/simulator/ModelDataManager.h"
#include "../../kernel/simulator/PluginInformation.h"
#include "Station.h"
#include "Label.h"
#include "AssignmentItem.h"

class SequenceStep : public PersistentObject_base {
public:
	SequenceStep(Station* station, std::list<Assignment*>* assignments = nullptr);
	SequenceStep(Label* label, std::list<Assignment*>* assignments = nullptr);
	SequenceStep(Model* model, std::string stationOrLabelName, bool isStation = true, std::list<Assignment*>* assignments = nullptr);
public: // virtual

	virtual bool _loadInstance(PersistenceRecord *fields, unsigned int parentIndex);
	virtual void _saveInstance(PersistenceRecord *fields, unsigned int parentIndex, bool saveDefaultValues);
	virtual bool _loadInstance(PersistenceRecord *fields);
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues);

public:

	std::list<Assignment*>* getAssignments() const;
	void setStation(Station* _station);
	Station* getStation() const;
	void setElementManager(ModelDataManager* _modeldataManager);
    void setLabel(Label* _label);
    Label* getLabel() const;
private:

	const struct DEFAULT_VALUES {
		const unsigned int assignmentsSize = 0;
	} DEFAULT;
	Station* _station = nullptr;
	Label* _label = nullptr;
	std::list<Assignment*>* _assignments = new std::list<Assignment*>();
private:
	ModelDataManager* _modeldataManager = nullptr;
};

/*!
 * Sequence module
DESCRIPTION
The Sequence module is used to define a sequence for entity flow through the model.
A sequence consists of an ordered list of stations that an entity will visit. For each
station in the visitation sequence, attributes and variables may be assigned values.
Each station in the visitation sequence is referred to as a step (or jobstep) in the
sequence.
Three special-purpose attributes are provided for all entities. The Sequence attribute
(Entity.Sequence) defines the sequence that an entity is to follow; a value of 0
indicates that the entity is not following any sequence. In order for an entity to follow
a sequence, its Sequence attribute must be assigned a value (for example, in the
Assign module).
The Jobstep attribute (Entity.Jobstep) stores the entity’s current step number in the
sequence. This value is updated automatically each time an entity is transferred. You
typically do not need to assign explicitly a value to Jobstep in the model.
The PlannedStation attribute (Entity.PlannedStation) stores the number of the station
associated with the next jobstep in the sequence. This attribute is not user-assignable. 
It is automatically updated whenever Entity.Sequence or Entity.JobStep changes, or
whenever the entity enters a station.
Jobstep names must be globally unique.
TYPICAL USES
* Define a routing path for part processing
* Define a sequence of steps patients must take upon arrival at an emergency room
 */
class Sequence : public ModelDataDefinition {
public:


public:
	Sequence(Model* model, std::string name = "");
	virtual ~Sequence() = default;
public:
	virtual std::string show();
public: // static 
	static PluginInformation* GetPluginInformation();
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
public:
	List<SequenceStep*>* getSteps() const;
protected:
	virtual bool _loadInstance(PersistenceRecord *fields);
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues);
	virtual bool _check(std::string* errorMessage);
private:
	List<SequenceStep*>* _steps = new List<SequenceStep*>();
};

#endif /* SEQUENCE_H */

