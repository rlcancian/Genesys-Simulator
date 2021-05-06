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

#include "Formula.h"
#include <iostream>
#include "ElementManager.h"
#include "Model.h"
#include "Traits.h"

Formula::Formula(Model* model, std::string name) : ModelElement(model, Util::TypeOf<Formula>(), name) {
	//_myPrivateParser = new Traits<Parser_if>::Implementation(_parentModel);
}

std::string Formula::show() {
	std::string expressions = "";
	unsigned int i = 0;
	// for (std::list<std::string>::iterator it = _formulaExpressions->list()->begin(); it != _formulaExpressions->list()->end(); it++) {
	//expressions += "expression[" + std::to_string(i++) + "]=\"" + (*it) + "\"; ";
	//}
	return ModelElement::show() + expressions;
}

unsigned int Formula::size() {
	return _formulaExpressions->size();
}

void Formula::setExpression(std::string index, std::string formulaExpression) {
	std::map<std::string, std::string>::iterator mapIt = _formulaExpressions->find(index);
	if (mapIt != _formulaExpressions->end()) {//found
		(*mapIt).second = formulaExpression;
	} else {//not found
		_formulaExpressions->insert({index, formulaExpression});
	}
}

std::string Formula::expression(std::string index) {
	std::map<std::string, std::string>::iterator it = _formulaExpressions->find(index);
	if (it == _formulaExpressions->end()) {
		return ""; // index does not exist. No formula expressions returned. \todo: Should it be traced?.
	} else {
		return it->second;
	}
}

void Formula::setExpression(std::string formulaExpression) {
	setExpression("", formulaExpression);
}

std::string Formula::expression() {
	return expression("");
}

double Formula::value(std::string index) {
	std::string strexpression = this->expression(index);
	double value = 0.0;
	try {
		value = _parentModel->parseExpression(strexpression);
	} catch (const std::exception& e) {
		_parentModel->getTracer()->traceError(e, "Error parsing formula \"" + strexpression + "\"");
	}
	return value;
}

double Formula::value() {
	return value("");
}

PluginInformation* Formula::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Formula>(), &Formula::LoadInstance);
	return info;
}

ModelElement* Formula::LoadInstance(Model* model, std::map<std::string, std::string>* fields) {
	Formula* newElement = new Formula(model);
	try {
		newElement->_loadInstance(fields);
	} catch (const std::exception& e) {

	}
	return newElement;
}

bool Formula::_loadInstance(std::map<std::string, std::string>* fields) {
	return ModelElement::_loadInstance(fields);
}

std::map<std::string, std::string>* Formula::_saveInstance() {
	std::map<std::string, std::string>* fields = ModelElement::_saveInstance();
	//SaveField(fields, "...", std::to_string(this->_...));
	return fields;
}

bool Formula::_check(std::string* errorMessage) {
	std::string errorMsg = "";
	bool res, resAll = true;
	//unsigned int i = 0;
	for (std::map<std::string, std::string>::iterator it = _formulaExpressions->begin(); it != _formulaExpressions->end(); it++) {
		res = _parentModel->checkExpression((*it).second, "formula expression[" + (*it).first + "]", &errorMsg);
		if (!res) {
			_parentModel->getTracer()->trace(Util::TraceLevel::L1_errorFatal, "Error parsing expression \"" + (*it).second + "\"");
		}
		resAll &= res;
	}
	return resAll;
}
