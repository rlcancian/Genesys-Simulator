/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   Failure.h
 * Author: rlcancian
 *
 * Created on 20 de Failureembro de 2019, 20:07
 */

#ifndef FAILURE_H
#define FAILURE_H


#include "kernel/simulator/ModelDataDefinition.h"
#include "kernel/simulator/ModelDataManager.h"
#include "kernel/simulator/PluginInformation.h"

class Resource;

/*!
Failure module
DESCRIPTION
The Failure module is designed for use with resources. When a failure occurs, the
entire resource (regardless of its capacity) is failed. Failures are designed to be used
with single-capacity resources or with multiple-capacity resources whose individual
resource units all fail at the same time.
TYPICAL USES
 * Breakdown information for a machine
 * Cash register tape refill every “x” customers
 * Random computer shutdowns or restarts
PROMPTS
Recordset Name of the recordset in the specified file from which to read
values. This field is available only if you specify a File Name
with a file access type, path, and recordset. Arena uses the Rows
and Columns properties to determine the amount of data to read
from the recordset. A recordset is required for all file types
except .xml. The recordset size must be equal to or greater than
the number of rows and columns specified for the expression.
Expression Values Lists the value or values of the expression. This property is not
available if you specify a File Name from which to read
expression values.
Expression Value Expression value associated with the expression name.
Prompt Description
Name The name of the failure associated with one or more resources.
Type Determines if the failure is time-based or count-based.
Count Defines the number of resource releases for count-based failures.
Valid when the Type is Count.
Up Time Defines the time between failures for time-based failures. Valid
when the Type is Time.
Up Time Units Time units for the time between failures (Up Time) for timebased failures.
Down Time Defines the duration of the failure.
Down Time Units Time units for the duration of the failure (Down Time).
Uptime in this State
only
Defines the state that should be considered for the time between
failures (only for time-based failures). If state is not specified,
then all states are considered (that is, the time between failures
does not depend on the time spent in a specific state, but rather
on the total simulation time). For example, you might want to
define a failure to be based only on the state Busy, and therefore,
the time between downtimes would be based on the amount of
time that a resource is busy, not simulated clock time.
 */
class Failure : public ModelDataDefinition {
public:

	enum class FailureType : int {
		COUNT = 0, TIME = 1, num_elements = 2
	};

	enum class FailureRule : int {
		IGNORE = 0, PREEMPT = 1, WAIT = 2, num_elements = 3
	};
public:
	static std::string convertEnumToStr(FailureType type);
	static std::string convertEnumToStr(FailureRule rule);
public:
	Failure(Model* model, std::string name = "");
	virtual ~Failure() override;
public: // static
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord *fields);
	static PluginInformation* GetPluginInformation();
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
public:
	virtual std::string show() override;
	void checkIsGoingToFailByCount(Resource* resource);
public: // gets & sets
	void setFailureType(FailureType _failureType);
	Failure::FailureType getFailureType() const;
	void setCountExpression(std::string countExpression);
	std::string getCountExpression() const;
	void setDownTimeTimeUnit(Util::TimeUnit downTimeTimeUnit);
	Util::TimeUnit getDownTimeTimeUnit() const;
	void setDownTimeExpression(std::string downTimeExpression);
	std::string getDownTimeExpression() const;
	void setUpTimeTimeUnit(Util::TimeUnit upTimeTimeUnit);
	Util::TimeUnit getUpTimeTimeUnit() const;
	void setUpTimeExpression(std::string upTimeExpression);
	std::string getUpTimeExpression() const;
	void setFailureRule(FailureRule _failureRule);
	FailureRule getFailureRule() const;

	List<Resource*>*falingResources() const;
	void addResource(Resource* newResource);
	void removeResource(Resource* resource);

protected: // must be overriden
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
protected: // could be overriden .
	virtual bool _check(std::string& errorMessage) override;
	virtual void _initBetweenReplications() override;
	//virtual void _createInternalAndAttachedData();
	//virtual ParserChangesInformation* _getParserChangesInformation();


protected:
	void _doCreateReportStatisticsDataDefinitions();
	void _doCreateEditableDataDefinitions();
	void _doCreateOthersDataDefinitions();


	void _createReportStatisticsDataDefinitions() override;
	void _createEditableDataDefinitions() override;
	void _createOthersDataDefinitions() override;

private: // simulation internal event handlers
	void _onFailureActiveEventHandler(void* resourcePtr);
	void _onFailureFailEventHandler(void* resourcePtr);
private:
	void _scheduleActivation(Resource* resource);
private:

	const struct DEFAULT_VALUES {
		const FailureType failureType = FailureType::TIME;
		const FailureRule failureRule = FailureRule::IGNORE;
		const std::string countExpression = "";
		const std::string upTimeExpression = "";
		const Util::TimeUnit upTimeTimeUnit = Util::TimeUnit::second;
		const std::string downTimeExpression = "";
		const Util::TimeUnit downTimeTimeUnit = Util::TimeUnit::second;
	} DEFAULT;
	FailureType _failureType = DEFAULT.failureType;
	FailureRule _failureRule = DEFAULT.failureRule;
	std::string _countExpression = DEFAULT.countExpression;
	std::string _upTimeExpression = DEFAULT.upTimeExpression;
	Util::TimeUnit _upTimeTimeUnit = DEFAULT.upTimeTimeUnit;
	std::string _downTimeExpression = DEFAULT.downTimeExpression;
	Util::TimeUnit _downTimeTimeUnit = DEFAULT.downTimeTimeUnit;
private:
	std::map<Resource*, unsigned int>* _releaseCounts = new std::map<Resource*, unsigned int>();
	List<Resource*>* _falingResources = new List<Resource*>();
};

#endif /* FAILURE_H */
