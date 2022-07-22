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
	void setDelayExpression(std::string _delayExpression);
	std::string delayExpression() const;
	void setDelay(double delay);
	double delay() const;
	void setDelayTimeUnit(Util::TimeUnit _delayTimeUnit);
	Util::TimeUnit delayTimeUnit() const;
public:
	virtual std::string show();
public:
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
protected:
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber);
	virtual bool _loadInstance(PersistenceRecord *fields);
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues);
protected:
	//virtual void _initBetweenReplications();
	virtual bool _check(std::string* errorMessage);
	virtual void _createInternalAndAttachedData();
public:
	const struct DEFAULT_VALUES {
		const std::string delayExpression = "1.0";
		const Util::TimeUnit delayTimeUnit = Util::TimeUnit::second;
	} DEFAULT;
private:
	std::string _delayExpression = DEFAULT.delayExpression;
	Util::TimeUnit _delayTimeUnit = DEFAULT.delayTimeUnit;
private: // inner internel elements
	StatisticsCollector* _cstatWaitTime = nullptr;
};

#endif /* DELAY_H */

