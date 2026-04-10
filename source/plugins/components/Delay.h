/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Delay.h
 * Author: rafael.luiz.cancian
 *
 * Created on 21 de Junho de 2018, 19:49
 */

#ifndef DELAY_H
#define DELAY_H

#include <string>
#include <vector>
#include "../../kernel/simulator/ModelComponent.h"
#include "../../kernel/simulator/Plugin.h"

/*!
Delay module
DESCRIPTION
The Delay module delays an entity by a specified amount of time.
When an entity arrives at a Delay module, the time delay expression is evaluated and
the entity remains in the module for the resulting time period. The time is then
allocated to the entity’s value-added, non-value added, transfer, wait, or other time.
Associated costs are calculated and allocated as well.
TYPICAL USES
* Processing a check at a bank
* Performing a setup on a machine
* Transferring a document to another department
PROMPTS
Prompt Description
Name Unique module identifier displayed on the module shape.
Allocation Type of category to which the entity’s incurred delay time and
cost will be added.
Delay Time Determines the value of the delay for the entity.
Units Time units used for the delay time. 
 */
class Delay : public ModelComponent {
public:
	Delay(Model* model, std::string name = "");
	virtual ~Delay() = default;
public:
	//void setDelayExpression(std::string _delayExpression);
	void setDelayExpression(std::string _delayExpression, Util::TimeUnit _delayTimeUnit=Util::TimeUnit::unknown);
	std::string delayExpression() const;
	void setDelay(double delay);
	double delay() const;
	void setDelayTimeUnit(Util::TimeUnit _delayTimeUnit);
	Util::TimeUnit delayTimeUnit() const;
    void setAllocation(Util::AllocationType allocation);
    Util::AllocationType getAllocation() const;
public:
	virtual std::string show() override;
public:
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
//public: // properties
	//using property_owner_t = Delay;
	//decl_property(DelayExpression, decl_get(std::string){return delayExpression();} void decl_set(std::string val){setDelayExpression(val);});

protected:
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override;
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
protected:
	//virtual void _initBetweenReplications();
	virtual bool _check(std::string& errorMessage) override;
	virtual void _createInternalAndAttachedData() override;
public:
	const struct DEFAULT_VALUES {
		const std::string delayExpression = "1.0";
		const Util::TimeUnit delayTimeUnit = Util::TimeUnit::second;
		const Util::AllocationType allocation = Util::AllocationType::Wait; 
		
	} DEFAULT;
private:
	std::string _delayExpression = DEFAULT.delayExpression;
	Util::TimeUnit _delayTimeUnit = DEFAULT.delayTimeUnit;
	Util::AllocationType _allocation = DEFAULT.allocation;
private:
	std::vector<std::string> _allAllocationAttachedAttributeNames() const;
	std::string _allocationAttachedAttributeName(Util::AllocationType allocation) const;
	void _reconcileAllocationAttachedAttributes();
private: // inner internal elements
	friend class DelayProbe;
	StatisticsCollector* _cstatWaitTime = nullptr;
};
//enable_this_owner(Delay, DelayExpression);

#endif /* DELAY_H */
