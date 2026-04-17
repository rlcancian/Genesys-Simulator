/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/class.h to edit this template
 */

/* 
 * File:   Schedule.h
 * Author: Prof. Rafael Luiz Cancian, Dr. Eng.
 *
 * Created on 14 de dezembro de 2022, 04:24
 */

#ifndef SCHEDULE_H
#define SCHEDULE_H

#include "kernel/simulator/ModelDataDefinition.h"
#include "kernel/simulator/PluginInformation.h"
#include "kernel/util/List.h"

class SchedulableItem {
public:

	enum class Rule : int {
		IGNORE = 1, PREEMPT = 2, WAIT = 3
	};

public:

    SchedulableItem(std::string expression, double duration, SchedulableItem::Rule rule = SchedulableItem::Rule::IGNORE) {
		this->expression = expression;
		this->duration = duration;
		this->rule = rule;
	}
public:

	void setDuration(double duration) {
		this->duration = duration;
	}

	double getDuration() const {
		return duration;
	}

	void setExpression(std::string expression) {
		this->expression = expression;
	}

	std::string getExpression() const {
		return expression;
	}

	void setRule(SchedulableItem::Rule rule) {
		this->rule = rule;
	}

	SchedulableItem::Rule getRule() const {
		return rule;
	}
private:
	std::string expression;
	double duration;
	SchedulableItem::Rule rule;
};

//class SchedulableResourceItem : public SchedulableItem {
//};

class Schedule : public ModelDataDefinition {
public:
	Schedule(Model* model, std::string name = "");
	virtual ~Schedule() override;
public: // static
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord *fields);
	static PluginInformation* GetPluginInformation();
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
public: // virtual
	virtual std::string show() override;
public:
	std::string getExpression();
	List<SchedulableItem*>* getSchedulableItems() const;
	void setRepeatAfterLast(bool _repeatAfterLast);
	bool isRepeatAfterLast() const;
protected: // must be overriden 
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
protected: // could be overriden by derived classes
	virtual bool _check(std::string& errorMessage) override;
	/*! This method returns all changes in the parser that are needed by plugins of this ModelDatas. When connecting a new plugin, ParserChangesInformation are used to change parser source code, whch is after compiled and dinamically linked to to simulator kernel to reflect the changes */
	virtual ParserChangesInformation* _getParserChangesInformation() override;
	virtual void _initBetweenReplications() override;
	/*! This method is necessary only for those components that instantiate internal elements that must exist before simulation starts and even before model checking. That's the case of components that have internal StatisticsCollectors, since others components may refer to them as expressions (as in "TVAG(ThisCSTAT)") and therefore the modeldatum must exist before checking such expression */
	virtual void _createInternalAndAttachedData() override; /*< A ModelDataDefinition or ModelComponent that includes (internal) ou refers to (attach) other ModelDataDefinition must register them inside this method. */
	virtual void _addProperty(SimulationControl* property) override;

private:

	const struct DEFAULT_VALUES {
		//const std::string someString = "Test";
		//const unsigned int someUint = 1;
		bool repeatAfterLast = true;
	} DEFAULT;
	bool _repeatAfterLast = DEFAULT.repeatAfterLast;
	List<SchedulableItem*>* _schedulableItems = new List<SchedulableItem*>();
	//std::string _someString = DEFAULT.someString;
	//unsigned int _someUint = DEFAULT.someUint;
};

#endif /* SCHEDULE_H */
