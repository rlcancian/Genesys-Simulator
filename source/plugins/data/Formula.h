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

#include "../../kernel/simulator/ModelDataDefinition.h"
#include "../../kernel/simulator/ModelDataManager.h"
#include "../../kernel/simulator/Plugin.h"
#include "../../kernel/simulator/Parser_if.h"

class Formula : public ModelDataDefinition {
public:
	Formula(Model* model, std::string name = "");
	virtual ~Formula() = default;
public: // virtual
	virtual std::string show();
public:
	unsigned int size();
	void setExpression(std::string index, std::string formulaExpression);
	void setExpression(std::string formulaExpression);
	std::string expression(std::string index);
	std::string expression();
	double value();
	double value(std::string index);
public: // statics
	static PluginInformation* GetPluginInformation();
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
protected: // must be overriden 
	virtual bool _loadInstance(PersistenceRecord *fields);
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues);
	virtual bool _check(std::string* errorMessage);
private:
private:
	std::map<std::string, std::string>* _formulaExpressions = new std::map<std::string, std::string>(); // map<index,formula>
};

#endif /* FORMULA_H */

