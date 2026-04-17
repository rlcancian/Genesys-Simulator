/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Formula.h
 * Author: rlcancian
 *
 * Created on 20 de Junho de 2019, 00:56
 */

#ifndef FORMULA_H
#define FORMULA_H

#include "kernel/simulator/ModelDataDefinition.h"
#include "kernel/simulator/ModelDataManager.h"
#include "kernel/simulator/Plugin.h"
//#include "kernel/simulator/Parser_if.h"

class Formula : public ModelDataDefinition {
public:
	Formula(Model* model, std::string name = "");
	virtual ~Formula() = default;
public: // virtual
	virtual std::string show() override;
public:
	unsigned int size();
	void setExpression(std::string formulaExpression, std::string index = "");
	//void setExpression(std::string formulaExpression);
	std::string getExpression(std::string index = "");
	//std::string getExpression();
	//double getValue();
	double getValue(std::string index);
public: // statics
	static PluginInformation* GetPluginInformation();
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
protected: // must be overriden by derived classes
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
protected: // could be overriden by derived classes
	virtual bool _check(std::string& errorMessage) override;
	/*! This method returns all changes in the parser that are needed by plugins of this ModelDatas. When connecting a new plugin, ParserChangesInformation are used to change parser source code, whch is after compiled and dinamically linked to to simulator kernel to reflect the changes */
	//virtual ParserChangesInformation* _getParserChangesInformation();
	//virtual void _initBetweenReplications();
	/*! This method is necessary only for those components that instantiate internal elements that must exist before simulation starts and even before model checking. That's the case of components that have internal StatisticsCollectors, since others components may refer to them as expressions (as in "TVAG(ThisCSTAT)") and therefore the modeldatum must exist before checking such expression */
	virtual void _createInternalAndAttachedData() override; /*< A ModelDataDefinition or ModelComponent that includes (internal) ou refers to (attach) other ModelDataDefinition must register them inside this method. */
	//virtual void _addProperty(PropertyBase* property);
private:
	std::map<std::string, std::string>* _formulaExpressions = new std::map<std::string, std::string>(); // map<index,formula>
private:
	///*static*/ Parser_if* _myPrivateParser;
};

#endif /* FORMULA_H */
