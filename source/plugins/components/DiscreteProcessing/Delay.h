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
#include "../../../kernel/simulator/model/ModelComponent.h"
#include "kernel/simulator/Plugin.h"

/*!
 * \brief Holds an entity in place for an evaluated expression's worth of
 * time, crediting that time to one of five allocation categories.
 *
 * Arena correspondence: the "Delay module" (Rockwell Automation, *Getting
 * Started with Arena*, "The Advanced Process Panel", pp. 53-54).
 * \c _allocation (a \c Util::AllocationType — ValueAdded/NonValueAdded/
 * Transfer/Wait/Others, matching Arena's five categories exactly) is the
 * closest correspondence to Arena's "Allocation" field.
 *
 * Confirmed behavior: `_onDispatchEvent()` credits the evaluated delay
 * **time** to the configured category twice — into a
 * `<EntityType>.<Category>Time` StatisticsCollector (when the EntityType
 * reports statistics) and into a running `Entity.Total<Category>Time`
 * attribute on the entity itself. This is the mechanism referenced by
 * `docs/ai_assistants/reference/ARENA_GENESYS_COMPATIBILITY.md` §6.3.
 *
 * Known difference from Arena: only **time** is allocated; no code path
 * converts accumulated category time into category **cost** (Arena's "costs
 * are calculated and allocated as well").
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
protected:
	virtual void _createInternalStatisticReporters() override;
	// virtual void _createNonEditableDataDefinitions() override;
	// virtual void _createEditableDataDefinitions() override;
	virtual void _createAttachedAttributes() override;
public:
	const struct DEFAULT_VALUES {
		const std::string delayExpression = "1.0";
		const Util::TimeUnit delayTimeUnit = Util::TimeUnit::second;
		const Util::AllocationType allocation = Util::AllocationType::Wait; 
		
	} DEFAULT;

protected:

private:
	std::string _delayExpression = DEFAULT.delayExpression;
	Util::TimeUnit _delayTimeUnit = DEFAULT.delayTimeUnit;
	Util::AllocationType _allocation = DEFAULT.allocation;
private:
	std::vector<std::string> _allAllocationAttachedAttributeNames() const;
	std::string _allocationAttachedAttributeName(Util::AllocationType allocation) const;
	void _reconcileAllocationAttachedAttributes();
	void _initCStats();
private: // inner internal elements
	friend class DelayProbe;
	StatisticsCollector* _cstatWaitTime = nullptr;
};
//enable_this_owner(Delay, DelayExpression);

#endif /* DELAY_H */