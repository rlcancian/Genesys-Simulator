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

#include "ModelElement.h"
#include "ElementManager.h"
#include "PluginInformation.h"
#include "Station.h"

class SequenceStep : PersistentObject_base {
public:

	class Assignment {
	public:

		Assignment(std::string destination, std::string expression) {
			this->_destination = destination;
			this->_expression = expression;
			// an assignment is always in the form:
			// (destinationType) destination = expression
		};

		void setDestination(std::string _destination) {
			this->_destination = _destination;
		}

		std::string getDestination() const {
			return _destination;
		}

		void setExpression(std::string _expression) {
			this->_expression = _expression;
		}

		std::string getExpression() const {
			return _expression;
		}
	private:
		std::string _destination = "";
		std::string _expression = "";

	};

public:

	SequenceStep(Station* station, std::list<Assignment*>* assignments = nullptr);
public: // virtual

	virtual bool _loadInstance(std::map<std::string, std::string>* fields, unsigned int parentIndex);
	virtual std::map<std::string, std::string>* _saveInstance(unsigned int parentIndex);
	virtual bool _loadInstance(std::map<std::string, std::string>* fields);
	virtual std::map<std::string, std::string>* _saveInstance();

public:

	std::list<SequenceStep::Assignment*>* getAssignments() const;
	void setStation(Station* _station);
	Station* getStation() const;
private:

	const struct DEFAULT_VALUES {
		const unsigned int assignmentsSize = 0;
	} DEFAULT;
	Station* _station;
	std::list<Assignment*>* _assignments;
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
 Define a routing path for part processing
 Define a sequence of steps patients must take upon arrival at an emergency room
 */
class Sequence : public ModelElement {
public:


public:
	Sequence(Model* model, std::string name = "");
	virtual ~Sequence() = default;
public:
	virtual std::string show();
public: // static 
	static PluginInformation* GetPluginInformation();
	static ModelElement* LoadInstance(Model* model, std::map<std::string, std::string>* fields);
	List<SequenceStep*>* getSteps() const;
public:
protected:
	virtual bool _loadInstance(std::map<std::string, std::string>* fields);
	virtual std::map<std::string, std::string>* _saveInstance();
	virtual bool _check(std::string* errorMessage);
private:
	List<SequenceStep*>* _steps = new List<SequenceStep*>();
};

#endif /* SEQUENCE_H */

