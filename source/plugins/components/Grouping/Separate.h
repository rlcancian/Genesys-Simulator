/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Separate.h
 * Author: rlcancian
 *
 * Created on 03 de Junho de 2019, 15:14
 */

#ifndef SEPARATE_H
#define SEPARATE_H

#include "../../../kernel/simulator/model/ModelComponent.h"

/*!
 * \brief Releases the original member entities of a temporary Batch group
 * back into the model.
 *
 * Arena correspondence: the "Separate module" (Rockwell Automation,
 * *Getting Started with Arena*, "The Basic Process Panel", p. 39), which
 * has two Types: Duplicate Original (clone the incoming entity N times,
 * optionally splitting VA/NVA/wait/transfer/other cost and time between the
 * copies) and Split Existing Batch (recover the original entities grouped
 * by Batch).
 *
 * Only Split Existing Batch is implemented here: `_onDispatchEvent()` reads
 * the `Entity.Group` marker attribute, looks up the matching EntityGroup
 * (created by Batch), and releases every original member with its group
 * marker cleared before removing the temporary representative entity.
 *
 * Known difference from Arena: there is no Duplicate Original mode — no
 * entity-cloning code path exists in this component at all. See
 * `docs/ai_assistants/reference/ARENA_GENESYS_COMPATIBILITY.md` §6.8; Clone
 * (`plugins/components/DiscreteProcessing/Clone.h`) may be the intended
 * substitute but has not been audited yet.
 */
class Separate : public ModelComponent {
public: // constructors
	Separate(Model* model, std::string name = "");
	virtual ~Separate() = default;
public: // virtual
	virtual std::string show() override;
public: // static
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
protected: // virtual
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override;
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
protected: // virtual
	//virtual void _initBetweenReplications();
	// virtual void _createInternalStatisticReporters() override;
	// virtual void _createNonEditableDataDefinitions() override;
	// virtual void _createEditableDataDefinitions() override;
	// virtual void _createAttachedAttributes() override;
	virtual bool _check(std::string& errorMessage) override;

protected:

private: // methods
private: // attributes 1:1
private: // attributes 1:n
};


#endif /* SEPARATE_H */
