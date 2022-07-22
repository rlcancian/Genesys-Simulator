/*
 * The MIT License
 *
 * Copyright 2022 rlcancian.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/* 
 * File:   AssignmentItem.cpp
 * Author: rlcancian
 * 
 * Created on 2 de fevereiro de 2022, 14:12
 */

#include "AssignmentItem.h"
#include "../../kernel/simulator/Attribute.h"
#include "../data/Variable.h"

Assignment::Assignment(Model* model, std::string destination, std::string expression, bool isAttributeNotVariable) {
	this->_destination = destination;
	this->_expression = expression;
	this->_isAttributeNotVariable = isAttributeNotVariable;
	if (isAttributeNotVariable) {
		if (model->getDataManager()->getDataDefinition(Util::TypeOf<Attribute>(), destination) == nullptr) {
			model->getDataManager()->insert(Util::TypeOf<Attribute>(), new Attribute(model, destination));
		}
	} else {
		if (model->getDataManager()->getDataDefinition(Util::TypeOf<Variable>(), destination) == nullptr) {
			model->getDataManager()->insert(Util::TypeOf<Variable>(), new Variable(model, destination));
		}
	}
}

Assignment::Assignment(std::string destination, std::string expression, bool isAttributeNotVariable) {
	this->_destination = destination;
	this->_expression = expression;
	this->_isAttributeNotVariable = isAttributeNotVariable;
	// an assignment is always in the form:
	// (destinationType) destination = expression
};

void Assignment::setDestination(std::string _destination) {
	this->_destination = _destination;
}

std::string Assignment::getDestination() const {
	return _destination;
}

void Assignment::setExpression(std::string _expression) {
	this->_expression = _expression;
}

std::string Assignment::getExpression() const {
	return _expression;
}

void Assignment::setAttributeNotVariable(bool isAttributeNotVariable) {
	this->_isAttributeNotVariable = isAttributeNotVariable;
}

bool Assignment::isAttributeNotVariable() const {
	return _isAttributeNotVariable;
}

bool Assignment::loadInstance(PersistenceRecord *fields, unsigned int parentIndex) {
	_destination = fields->loadField("assignDest" + strIndex(parentIndex), "");
	_expression = fields->loadField("assignExpr" + strIndex(parentIndex), "");
	_isAttributeNotVariable = fields->loadField("assignIsAttrib" + strIndex(parentIndex), true);
	return true;
}

void Assignment::saveInstance(PersistenceRecord *fields, unsigned int parentIndex, bool saveDefaultValues) {
	std::string num = strIndex(parentIndex);
	fields->saveField("assignDest" + strIndex(parentIndex), getDestination(), "", saveDefaultValues);
	fields->saveField("assignExpr" + strIndex(parentIndex), getExpression(), "", saveDefaultValues);
	fields->saveField("assignIsAttrib" + strIndex(parentIndex), isAttributeNotVariable(), true, saveDefaultValues);
}
