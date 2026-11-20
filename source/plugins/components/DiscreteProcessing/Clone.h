/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Clone.h
 * Author: rafael.luiz.cancian
 *
 * Created on 30 de Novembro de 2021, 18:50
 */

#ifndef CLONE_H
#define CLONE_H

#include "../../../kernel/simulator/model/ModelComponent.h"
#include "../../../kernel/simulator/essentialPlugins/Counter.h"

/*!
 * \brief Creates `_numClonesExpression` copies of the incoming entity
 * (same EntityType and Attribute values), sending clones out port 1 and the
 * original out port 0.
 *
 * Arena correspondence: confirmed by the maintainer (2026-08-30) as the
 * intended GenESyS counterpart to the "Duplicate Original" Type of Arena's
 * "Separate module" (Rockwell Automation, *Getting Started with Arena*,
 * "The Basic Process Panel", p. 39) — see Separate, whose own
 * `_onDispatchEvent()` only implements Arena's other Type ("Split Existing
 * Batch"). `_numClonesExpression` matches Arena's "# of Duplicates".
 *
 * Known difference from Arena: no "Percent Cost to Duplicates" — splitting
 * accumulated category cost/time between the original and its clones is not
 * implemented (see
 * `docs/ai_assistants/reference/ARENA_GENESYS_COMPATIBILITY.md` §6.3 and
 * §6.19).
 */
class Clone : public ModelComponent {
public: // constructors
	Clone(Model* model, std::string name = "");
	virtual ~Clone() = default;
public: // virtual
	virtual std::string show() override;
public: // 
	void setNumClonesExpression(std::string numClones);
	std::string getNumClonesExpression() const;
public: // static
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
protected: // must be overriden 
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override;
protected: // could be overriden .
	virtual bool _check(std::string& errorMessage) override;
	virtual void _initBetweenReplications() override;
	// virtual void _createInternalAndAttachedData() override;
	//virtual ParserChangesInformation* _getParserChangesInformation();

protected:
	virtual void _createInternalStatisticReporters() override;
	// virtual void _createNonEditableDataDefinitions() override;
	// virtual void _createEditableDataDefinitions() override;
	// virtual void _createAttachedAttributes() override;

private: // methods
private: // attributes 1:1

	const struct DEFAULT_VALUES {
		const std::string numClonesExpression = "1";
	} DEFAULT;
	std::string _numClonesExpression = DEFAULT.numClonesExpression;
	Counter* _counter = nullptr;
private: // attributes 1:n
};

#endif /* CLONE_H */
