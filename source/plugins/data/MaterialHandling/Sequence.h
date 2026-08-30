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

#include "../../../kernel/simulator/model/ModelDataDefinition.h"
#include "../../../kernel/simulator/model/ModelDataManager.h"
#include "kernel/simulator/PluginInformation.h"
#include "Station.h"
#include "../Logic/Label.h"
#include "../Logic/AssignmentItem.h"

class SequenceStep : public PersistentObject_base {
public:
	SequenceStep(Station* station, std::list<Assignment*>* assignments = nullptr);
	SequenceStep(Label* label, std::list<Assignment*>* assignments = nullptr);
	SequenceStep(Model* model, std::string stationOrLabelName, bool isStation = true, std::list<Assignment*>* assignments = nullptr);
	virtual ~SequenceStep() override;
public: // virtual

	virtual bool _loadInstance(PersistenceRecord *fields, unsigned int parentIndex);
	virtual void _saveInstance(PersistenceRecord *fields, unsigned int parentIndex, bool saveDefaultValues);
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;

public:

	std::list<Assignment*>* getAssignments() const;
	void setStation(Station* _station);
	Station* getStation() const;
	void setElementManager(ModelDataManager* _modeldataManager);
    void setLabel(Label* _label);
    Label* getLabel() const;

protected:
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
 * \brief Data definition holding an ordered list of visitation steps
 * (\c SequenceStep), each naming a Station or Label an entity should reach
 * next, with optional attribute/variable assignments per step.
 *
 * Arena correspondence: the "Sequence module" (Rockwell Automation,
 * *Getting Started with Arena*, "The Advanced Transfer Panel", pp. 100-101),
 * which defines an ordered visitation list of stations (jobsteps) plus
 * per-step attribute/variable/picture assignments, tracked at runtime
 * through the special-purpose Entity.Sequence/Entity.Jobstep/
 * Entity.PlannedStation attributes.
 *
 * GenESyS extends the concept by letting a SequenceStep target a Label
 * instead of a Station (useful outside pure station-to-station material
 * handling flows).
 *
 * Known difference from Arena, to confirm in \c Sequence.cpp: steps are
 * stored as a plain ordered \c List, so Arena's named, out-of-order "Step
 * Name"/"Next Step" jump targets have no obvious equivalent here.
 */
class Sequence : public ModelDataDefinition {
public:


public:
	Sequence(Model* model, std::string name = "");
	virtual ~Sequence() override;
public:
	virtual std::string show() override;
public: // static 
	static PluginInformation* GetPluginInformation();
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
public:
	List<SequenceStep*>* getSteps() const;
protected:
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
	virtual bool _check(std::string& errorMessage) override;
protected:
	// virtual void _createInternalStatisticReporters() override;
	// virtual void _createNonEditableDataDefinitions() override;
	// virtual void _createEditableDataDefinitions() override;
	virtual void _createAttachedAttributes() override;
private:
	List<SequenceStep*>* _steps = new List<SequenceStep*>();
};

#endif /* SEQUENCE_H */
