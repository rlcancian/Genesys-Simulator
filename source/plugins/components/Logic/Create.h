/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Create.h
 * Author: rafael.luiz.cancian
 *
 * Created on 21 de Junho de 2018, 20:12
 */

#ifndef CREATE_H
#define CREATE_H

#include <string>
#include <limits>
#include "kernel/simulator/SourceModelComponent.h"
#include "../../../kernel/simulator/essentialPlugins/EntityType.h"
#include "../../../kernel/simulator/essentialPlugins/Counter.h"
#include "kernel/simulator/Plugin.h"

#include "../../data/Logic/Formula.h"
#include "plugins/data/DiscreteProcessing/Schedule.h"

/*!
 * \brief Source component that generates entities and starts them into the
 * model, driven by a parsed inter-arrival expression, a Schedule, or a
 * Formula.
 *
 * Arena correspondence: the "Create module" (Rockwell Automation, *Getting
 * Started with Arena*, "The Basic Process Panel", pp. 31-32). Entity Type,
 * First Creation, Entities per Arrival and Max Arrivals are generalized on
 * the base class SourceModelComponent; see that header for their accessors.
 * Arena's Type field (Random/Schedule/Constant/Expression) is not a stored
 * enum here — Random/Constant/Expression collapse into one parsed
 * time-between-creations expression (SourceModelComponent's default
 * `"EXPO(1.0)"` already matches Arena's default Random behavior), while
 * `_timeBetweenCreationsSchedule` keeps Schedule as a separate, mutually
 * exclusive alternative (\c _check() enforces exactly one active source).
 * `_timeBetweenCreationsFormula` adds a Formula-backed alternative beyond
 * what Arena exposes.
 *
 * See `docs/ai_assistants/reference/ARENA_GENESYS_COMPATIBILITY.md` §6.1 for
 * the full audited comparison, including an open question about whether
 * Arena's VA/NVA/Wait/Transfer/Other per-entity cost/time accounting has any
 * GenESyS runtime equivalent.
 */
class Create : public SourceModelComponent {
public:
	Create(Model* model, std::string name = "");
	virtual ~Create() = default;
public: // virtual
	virtual std::string show() override;
public:
	void setTimeBetweenCreationsFormula(Formula* _timeBetweenCreationsFormula);
	Formula* getTimeBetweenCreationsFormula() const;
	void setTimeBetweenCreationsSchedule(Schedule* _timeBetweenCreationsSchedule);
	Schedule* getTimeBetweenCreationsSchedule() const;
public: // static
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

	double testePropertyCreateDouble() const;
	void setTestePropertyCreateDouble(double newTestePropertyCreateDouble);

protected:
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override;
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _initBetweenReplications() override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
	virtual bool _check(std::string& errorMessage) override;
	virtual void _createInternalStatisticReporters() override;
	virtual void _createEditableDataDefinitions() override;
private:
	double _lastArrival = -1.0;
private: // internal elements
	Counter* _numberOut = nullptr; // internal modeldatum
private: // attached elements
	Schedule* _timeBetweenCreationsSchedule = nullptr;
	Formula* _timeBetweenCreationsFormula = nullptr;
};

#endif /* CREATE_H */
