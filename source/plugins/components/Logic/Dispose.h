/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Dispose.h
 * Author: rafael.luiz.cancian
 *
 * Created on 21 de Junho de 2018, 20:13
 */

#ifndef DISPOSE_H
#define DISPOSE_H

#include "kernel/simulator/SinkModelComponent.h"
#include "../../../kernel/simulator/essentialPlugins/Counter.h"
#include "kernel/simulator/Plugin.h"

/*!
 * \brief Sink component that ends an entity's life in the model, optionally
 * recording its total time in system before removing it.
 *
 * Arena correspondence: the "Dispose module" (Rockwell Automation, *Getting
 * Started with Arena*, "The Basic Process Panel", p. 32). \c
 * isReportStatistics() (inherited) is the closest correspondence to Arena's
 * "Record Entity Statistics" checkbox.
 *
 * Known difference from Arena: only a single collapsed
 * \c TotalTimeInSystem statistic is produced per EntityType (gated by that
 * EntityType's own \c isReportStatistics()); Arena's separate
 * value-added/non-value-added/wait/transfer/other time-and-cost breakdown
 * has no confirmed GenESyS runtime equivalent — see
 * `docs/ai_assistants/reference/ARENA_GENESYS_COMPATIBILITY.md` §6.2-§6.3.
 */
class Dispose : public SinkModelComponent {
public:
	Dispose(Model* model, std::string name = "");
	virtual ~Dispose() = default;
public:
	virtual std::string show() override;
public:
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
protected:
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override;
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
protected:
	virtual void _initBetweenReplications() override;
	virtual bool _check(std::string& errorMessage) override;
protected:
	virtual void _createInternalStatisticReporters() override;
	// virtual void _createNonEditableDataDefinitions() override;
	// virtual void _createEditableDataDefinitions() override;
	virtual void _createAttachedAttributes() override;
protected:
private: // internal elements
	Counter* _numberOut = nullptr;
};

#endif /* DISPOSE_H */
