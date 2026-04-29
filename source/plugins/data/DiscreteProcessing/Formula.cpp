/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Formula.cpp
 * Author: rlcancian
 * 
 * Created on 20 de Junho de 2019, 00:56
 */

#include "plugins/data/DiscreteProcessing/Formula.h"
#include <iostream>
#include "kernel/simulator/ModelDataManager.h"
#include "kernel/simulator/Model.h"
#include "kernel/TraitsKernel.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &Formula::GetPluginInformation;
}
#endif

//Parser_if* Formula::_myPrivateParser = new TraitsKernel<Parser_if>::Implementation(model, sampler);

ModelDataDefinition* Formula::NewInstance(Model* model, std::string name) {
	return new Formula(model, name);
}

Formula::Formula(Model* model, std::string name) : ModelDataDefinition(model, Util::TypeOf<Formula>(), name) {
	//_myPrivateParser = new TraitsKernel<Parser_if>::Implementation(_parentModel, new TraitsKernel<Sampler_if>::Implementation()); //@TODO: !! Really, really BAD. 
}

std::string Formula::show() {
	std::string expressions = "";
	//unsigned int i = 0;
	// for (std::list<std::string>::iterator it = _formulaExpressions->list()->begin(); it != _formulaExpressions->list()->end(); it++) {
	//expressions += "expression[" + std::to_string(i++) + "]=\"" + (*it) + "\"; ";
	//}
	return ModelDataDefinition::show() + expressions;
}

unsigned int Formula::size() {
	return _formulaExpressions->size();
}

void Formula::setExpression(std::string formulaExpression, std::string index) {
	std::map<std::string, std::string>::iterator mapIt = _formulaExpressions->find(index);
	if (mapIt != _formulaExpressions->end()) {//found
		(*mapIt).second = formulaExpression;
	} else {//not found
		_formulaExpressions->insert({index, formulaExpression});
	}
}

std::string Formula::getExpression(std::string index) {
	std::map<std::string, std::string>::iterator it = _formulaExpressions->find(index);
	if (it == _formulaExpressions->end()) {
		return ""; // index does not exist. No formula expressions returned. @TODO: Should it be traced?.
	} else {
		return it->second;
	}
}

double Formula::getValue(std::string index) {
	std::string strexpression = this->getExpression(index);
	double value = 0.0;
	try {
		value = _parentModel->parseExpression(strexpression);
		//value = this->_myPrivateParser->parse(strexpression); // @TODO: must create another wrapper / driver
	} catch (const std::exception& e) {
		traceError("Error parsing formula \"" + strexpression + "\"", e);
	}
	return value;
}

PluginInformation* Formula::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Formula>(), &Formula::LoadInstance, &Formula::NewInstance);
	return info;
}

ModelDataDefinition* Formula::LoadInstance(Model* model, PersistenceRecord *fields) {
	Formula* newElement = new Formula(model);
	try {
		newElement->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newElement;
}

bool Formula::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelDataDefinition::_loadInstance(fields);
	if (res) {
		/*!
		 * \brief Load formula expressions map.
		 */
		_formulaExpressions->clear();
		unsigned int expressions = fields->loadField("expressions", 0u);
		for (unsigned int i = 0; i < expressions; i++) {
			std::string suffix = Util::StrIndex(i);
			std::string index = fields->loadField("exprIndex" + suffix, "");
			std::string expression = fields->loadField("exprValue" + suffix, "");
			_formulaExpressions->insert({index, expression});
		}
	}
	return res;
}

void Formula::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelDataDefinition::_saveInstance(fields, saveDefaultValues);
	/*!
	 * \brief Persist formula expressions map.
	 */
	fields->saveField("expressions", _formulaExpressions->size(), 0u, saveDefaultValues);
	unsigned int i = 0;
	for (const auto& pair : *_formulaExpressions) {
		std::string suffix = Util::StrIndex(i);
		fields->saveField("exprIndex" + suffix, pair.first, "", saveDefaultValues);
		fields->saveField("exprValue" + suffix, pair.second, "", saveDefaultValues);
		i++;
	}
}

bool Formula::_check(std::string& errorMessage) {
	bool res, resAll = true;
	//unsigned int i = 0;
	for (std::map<std::string, std::string>::iterator it = _formulaExpressions->begin(); it != _formulaExpressions->end(); it++) {
		res = _parentModel->checkExpression((*it).second, "formula expression[" + (*it).first + "]", errorMessage);
		if (!res) {
			traceError("Error parsing expression \"" + (*it).second + "\"");
		}
		resAll &= res;
	}
	return resAll;
}

void Formula::_createInternalAndAttachedData() {
	this->_attachedDataInsert("Itself", this); // @TODO: Self reference to avoid it to be excluded during orphan checking, since no comonent knows its expression may have a formula
}

void Formula::_createReportStatisticsDataDefinitions() {
}

void Formula::_createEditableDataDefinitions() {
}

void Formula::_createOthersDataDefinitions() {
}
