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
 * Author: Prof. Rafael Luiz Cancian, Dr. Eng.
 * 
 * Created on 2 de fevereiro de 2022, 14:12
 */

#include "plugins/data/DiscreteProcessing/AssignmentItem.h"
#include "kernel/simulator/Attribute.h"
#include "plugins/data/DiscreteProcessing/Variable.h"

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
	_updateTypeDC();

	SimulationControlGeneric<std::string>* propDestination = new SimulationControlGeneric<std::string>(
				std::bind(&Assignment::getDestination, this),
				std::bind(&Assignment::setDestination, this, std::placeholders::_1),
				Util::TypeOf<Assignment>(), getName(), "Destination", "");
	SimulationControlGeneric<std::string>* propExpression = new SimulationControlGeneric<std::string>(
				std::bind(&Assignment::getExpression, this),
				std::bind(&Assignment::setExpression, this, std::placeholders::_1),
				Util::TypeOf<Assignment>(), getName(), "Expression", "");
	SimulationControlGeneric<bool>* propAttributeNotVariable = new SimulationControlGeneric<bool>(
				std::bind(&Assignment::isAttributeNotVariable, this),
				std::bind(&Assignment::setAttributeNotVariable, this, std::placeholders::_1),
				Util::TypeOf<Assignment>(), getName(), "AttributeNotVariable", "");

	model->getControls()->insert(propDestination);
	model->getControls()->insert(propExpression);
	model->getControls()->insert(propAttributeNotVariable);

	// setting properties
    _addSimulationControl(propDestination);
    _addSimulationControl(propExpression);
	_addSimulationControl(propAttributeNotVariable);
}

Assignment::Assignment(std::string destination, std::string expression, bool isAttributeNotVariable) {
	this->_destination = destination;
	this->_expression = expression;
	this->_isAttributeNotVariable = isAttributeNotVariable;
	// an assignment is always in the form:
	// (destinationType) destination = expression
	_updateTypeDC();
};

Assignment::~Assignment() {
    delete _simulationControls;
}

void Assignment::setDestination(std::string _destination) {
	this->_destination = _destination;
}

std::string Assignment::getDestination() const {
	return _destination;
}

std::string Assignment::getName() const {
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
	_updateTypeDC();
}

bool Assignment::isAttributeNotVariable() const {
	return _isAttributeNotVariable;
}

void Assignment::_addSimulationControl(SimulationControl* control) {
    _simulationControls->insert(control);
}

List<SimulationControl*>* Assignment::getSimulationControls() const {
    return _simulationControls;
}

bool Assignment::loadInstance(PersistenceRecord *fields, unsigned int parentIndex) {
	_destination = fields->loadField("assignDest" + Util::StrIndex(parentIndex), "");
	_expression = fields->loadField("assignExpr" + Util::StrIndex(parentIndex), "");
	_isAttributeNotVariable = fields->loadField("assignIsAttrib" + Util::StrIndex(parentIndex), true);
	_updateTypeDC();
	return true;
}

void Assignment::saveInstance(PersistenceRecord *fields, unsigned int parentIndex, bool saveDefaultValues) {
	fields->saveField("assignDest" + Util::StrIndex(parentIndex), getDestination(), "", saveDefaultValues);
	fields->saveField("assignExpr" + Util::StrIndex(parentIndex), getExpression(), "", saveDefaultValues);
	fields->saveField("assignIsAttrib" + Util::StrIndex(parentIndex), isAttributeNotVariable(), true, saveDefaultValues);
}

void Assignment::_updateTypeDC() {
	if (_isAttributeNotVariable) {
		_typeDC = Util::TypeOf<Attribute>();
	} else {
		_typeDC = Util::TypeOf<Variable>();
	}
}
