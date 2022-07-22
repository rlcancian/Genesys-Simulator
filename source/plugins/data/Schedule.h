/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Schedule.h
 * Author: rlcancian
 *
 * Created on 03 de Junho de 2019, 15:16
 */

#ifndef SCHEDULE_H
#define SCHEDULE_H

class Model;

/*!
 Schedule module
DESCRIPTION
This data module may be used in conjunction with the Resource module to define an
operating schedule for a resource or with the Create module to define an arrival
schedule. Additionally, a schedule may be used and referenced to factor time delays
based on the simulation time.
TYPICAL USES
* Work schedule for staff, including breaks
* Breakdown patterns for equipment
* Volume of customers arriving at a store
* Learning-curve factors for new workers
PROMPTS
File Read Time Specifies when to read the values from the file into the variable.
If you select PreCheck, the values for the variable are read while
the model is still in Edit mode (prior to the model being checked
and compiled). If you select BeginSimulation, values are read
when the model is compiled, prior to the first replication. If you
select BeginReplication, values are read prior to each
replication.
Initial Values Lists the initial value or values of the variable. You can assign
new values to the variable at different stages of the model by
using the Assign module.
Initial Value Variable value at the start of the simulation.
Prompt Description
Name The name of the schedule being defined. This name must be
unique.
Type Type of schedule being defined. This may be Capacity-related
(for resource schedules), Arrival-related (for the Create
module), or Other (miscellaneous time delays or factors)
Time Units Time units used for the time-duration information.
Scale Factor Method of scaling the schedule for increases or decreases in
Arrival/Other values. The specified Value fields will be
multiplied by the scale factor to determine the new values. Not
available for Capacity-type schedules.
Durations Lists the value and duration pairs for the schedule. Values can be
capacity, arrival, or other type values, while the duration is
specified in time units. Schedule pairs will repeat after all
durations have been completed, unless the last duration is left
blank (infinite). Schedule data can be entered graphically using
the graphical schedule editor or manually using the Value/
Duration fields.
Value Represents either the capacity of a resource (if Type is Capacity),
arrival rate (if Type is Arrival), or some other value (if Type is
Other). Examples of Other may be a factor that is used in a delay
expression to scale a delay time during various parts of the day.
Duration Time duration for which a specified Value will be valid.
 */

#include "../../kernel/simulator/ModelDataDefinition.h"
#include "../../kernel/simulator/ModelDataManager.h"
#include "../../kernel/simulator/PluginInformation.h"

class Schedule : public ModelDataDefinition {
public: // constructors
	Schedule(Model* model, std::string name = "");
	virtual ~Schedule() = default;
public: // virtual
	virtual std::string show();
public: // static 
	static PluginInformation* GetPluginInformation();
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
public:
protected:
	virtual bool _loadInstance(PersistenceRecord *fields);
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues);
	virtual bool _check(std::string* errorMessage);
private:
};

#endif /* SCHEDULE_H */

