/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Assign.h
 * Author: rafael.luiz.cancian
 *
 * Created on 31 de Agosto de 2018, 10:10
 */

#ifndef ASSIGN_H
#define ASSIGN_H

#include "../../../kernel/simulator/model/ModelComponent.h"
#include "../../../kernel/simulator/model/Model.h"
#include "kernel/simulator/Plugin.h"
#include "../../../kernel/simulator/essentialPlugins/Attribute.h"
#include "../../data/Logic/AssignmentItem.h"
#include "../../data/Logic/Variable.h"

/*!
 * \brief Component that evaluates one or more (destination, expression)
 * pairs and writes each result into an Attribute or a Variable on the
 * entity/model.
 *
 * Arena correspondence: the "Assign module" (Rockwell Automation, *Getting
 * Started with Arena*, "The Basic Process Panel", pp. 40-41), which supports
 * five assignment kinds: Attribute, Variable (optionally a 1D/2D array via
 * Row/Column), Entity Type, Entity Picture, and Other (system variables such
 * as resource capacity or simulation end time).
 *
 * Each entry in \c _assignments (see Assignment, in
 * `plugins/data/Logic/AssignmentItem.h`) only distinguishes Attribute vs.
 * Variable destinations; an optional `name[index]` syntax on the
 * destination string is the closest correspondence to Arena's separate
 * Row/Column fields.
 *
 * Known difference from Arena: there is no Entity Type, Entity Picture, or
 * Other (system-variable) assignment kind — `_onDispatchEvent()` only has
 * Attribute and Variable branches. See
 * `docs/ai_assistants/reference/ARENA_GENESYS_COMPATIBILITY.md` §6.4.
 */
class Assign : public ModelComponent {
public:
	Assign(Model* model, std::string name = "");
	virtual ~Assign() override;
public:
	virtual std::string show() override;
public:
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
public:
	List<Assignment*>* getAssignments() const;
	void addAssignment(Assignment* newAssignment);
	void removeAssignment(Assignment* assignment);
protected: // must be overriden 
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override;
protected: // could be overriden by derived classes
	virtual bool _check(std::string& errorMessage) override;
	/*! This method returns all changes in the parser that are needed by plugins of this ModelDatas. When connecting a new plugin, ParserChangesInformation are used to change parser source code, whch is after compiled and dinamically linked to to simulator kernel to reflect the changes */
	//virtual ParserChangesInformation* _getParserChangesInformation();
	//virtual void _initBetweenReplications();
protected:
	// virtual void _createInternalStatisticReporters() override;
	// virtual void _createNonEditableDataDefinitions() override;
	virtual void _createEditableDataDefinitions() override;
	//virtual void _createAttachedAttributes() override;
	/*! This method is necessary only for those components that instantiate internal elements that must exist before simulation starts and even before model checking. That's the case of components that have internal StatisticsCollectors, since others components may refer to them as expressions (as in "TVAG(ThisCSTAT)") and therefore the modeldatum must exist before checking such expression */
	// virtual void _createInternalAndAttachedData() override;
	//virtual void _addSimulationControl(SimulationControl* property);

protected:

private:
	static std::string _destinationBaseName(const std::string& destination);
	std::string _destinationIndex(const std::string& destination);
	void _prepareAssignment(Assignment* assignment);
private:

	const struct DEFAULT_VALUES {
		const unsigned int assignmentsSize = 1;
	} DEFAULT;
	List<Assignment*>* _assignments = new List<Assignment*>();
};

#endif /* ASSIGN_H */
