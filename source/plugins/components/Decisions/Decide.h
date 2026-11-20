/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Decide.h
 * Author: rafael.luiz.cancian
 *
 * Created on 9 de Agosto de 2018, 20:39
 */

#ifndef DECIDE_H
#define DECIDE_H

#include "../../../kernel/simulator/model/ModelComponent.h"
#include "../../../kernel/simulator/essentialPlugins/Counter.h"
#include <deque>
#include <utility>

/*!
 * \brief Routes an entity to the first output port whose parser expression
 * evaluates truthy, or to a trailing "else" port when none do.
 *
 * Arena correspondence: the "Decide module" (Rockwell Automation, *Getting
 * Started with Arena*, "The Basic Process Panel", pp. 36-37), which offers a
 * structured 2-way/N-way, by-Condition/by-Chance dialog (Named/Is/Value with
 * Row/Column for array conditions, or a Percentages field for Chance
 * branching).
 *
 * GenESyS collapses all of that into one ordered list of boolean parser
 * expressions (\c _conditions), one per output port — a deliberate
 * generalization, not a missing capability. There is no dedicated
 * percentage-based Chance input; expressing chance-based branching (e.g.
 * "60% true") is left to the modeler as a boolean expression over a random
 * draw. Each port has its own entity-count Counter when reporting is
 * enabled.
 */
class Decide : public ModelComponent {
public:
	struct ConditionEdit {
		enum class Kind { Insert, Replace, Remove, Move };
		Kind kind;
		unsigned int index = 0;
		std::string before;
		std::string after;
		ModelComponent* removedTarget = nullptr;
		unsigned int removedTargetPort = 0;
		unsigned int destinationIndex = 0;
	};

	Decide(Model* model, std::string name = "");
	virtual ~Decide() override;
public:
	List<std::string>* getConditions() const;
	void addConditions(std::string newCondition);
	void insertConditionAt(unsigned int index, std::string condition);
	void replaceConditionAt(unsigned int index, std::string condition);
	void moveCondition(unsigned int from, unsigned int to);
	void removeConditionAt(unsigned int index);
	void editConditionInsertAt(unsigned int index, std::string condition);
	void editConditionReplaceAt(unsigned int index, std::string condition);
	void editConditionRemoveAt(unsigned int index);
	void removeConditions(std::string condition);
	void synchronizeOutputStatistics();
	bool takeConditionPortEdit(unsigned int& index, bool& inserted);
	bool takeConditionEdit(ConditionEdit& edit);
	void markConditionPortEdit(unsigned int index, bool inserted);
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
	//virtual void _initBetweenReplications();
	virtual bool _check(std::string& errorMessage) override;
protected:
	virtual void _createInternalStatisticReporters() override;
	// virtual void _createNonEditableDataDefinitions() override;
	// virtual void _createEditableDataDefinitions() override;
	// virtual void _createAttachedAttributes() override;

protected:

private:
	List<std::string>* _conditions = new List<std::string>();
	List<Counter*>* _numberOuts = nullptr;
	std::deque<std::pair<unsigned int, bool>> _pendingConditionPortEdits;
	std::deque<ConditionEdit> _pendingConditionEdits;
private:

};

#endif /* DECIDE_H */
