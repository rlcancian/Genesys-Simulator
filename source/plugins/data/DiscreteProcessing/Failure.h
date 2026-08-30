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


#include "../../../kernel/simulator/model/ModelDataDefinition.h"
#include "../../../kernel/simulator/model/ModelDataManager.h"
#include "kernel/simulator/PluginInformation.h"

class Resource;

/*!
 * \brief Data definition describing a resource breakdown pattern (whole
 * resource failure regardless of capacity) attached to one or more
 * Resource instances.
 *
 * Arena correspondence: the "Failure module" (Rockwell Automation, *Getting
 * Started with Arena*, "The Advanced Process Panel", pp. 71-72). \c
 * FailureType (COUNT/TIME) mirrors Arena's count-based/time-based failure
 * types; \c _countExpression, \c _upTimeExpression/_upTimeTimeUnit and \c
 * _downTimeExpression/_downTimeTimeUnit are the closest correspondence to
 * Arena's Count, Up Time and Down Time fields (generalized to parsed
 * expressions rather than static numbers). \c FailureRule
 * (IGNORE/PREEMPT/WAIT) mirrors Arena's Failure Rule as configured per
 * resource/failure pair.
 *
 * Known difference from Arena: there is no "Uptime in this State only"
 * field, so a time-based failure's up-time clock cannot be scoped to a
 * single resource state (e.g. only while Busy); it always runs against
 * total simulated time.
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
	//virtual ParserChangesInformation* _getParserChangesInformation();
protected:
	// virtual void _createInternalStatisticReporters() override;
	// virtual void _createNonEditableDataDefinitions() override;
	// virtual void _createEditableDataDefinitions() override;
	// virtual void _createAttachedAttributes() override;

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
