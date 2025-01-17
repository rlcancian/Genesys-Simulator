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
 * File:   AssignmentItem.h
 * Author: rlcancian
 *
 * Created on 2 de fevereiro de 2022, 14:12
 */

#ifndef ASSIGNMENTITEM_H
#define ASSIGNMENTITEM_H

#include "../../kernel/simulator/Model.h"

/*!
 * While the assign ans sequence classes allows you to perform multiple assignments, the assignmentItem class defines an assignment itself.
 */
class Assignment {
public:
	Assignment(Model* model, std::string destination, std::string expression="1", bool isAttributeNotVariable = true);
	Assignment(std::string destination, std::string expression, bool isAttributeNotVariable = true);
	void setDestination(std::string _destination);
	std::string getDestination() const;
	void setExpression(std::string _expression);
	std::string getExpression() const;
	void setAttributeNotVariable(bool isAttributeNotVariable);
	bool isAttributeNotVariable() const;

	// TODO: don't have name
	std::string getName() const;

	List<PropertyBase*>* getProperties() const;
    void _addProperty(PropertyBase* property);

	std::string getTypeDC() {return _typeDC;};
public:
	bool loadInstance(PersistenceRecord *fields, unsigned int parentIndex);
	void saveInstance(PersistenceRecord *fields, unsigned int parentIndex, bool saveDefault);
private:
	std::string _destination = "";
	std::string _expression = "";
	bool _isAttributeNotVariable = true;
	std::string _typeDC;

	List<PropertyBase*>* _properties = new List<PropertyBase*>();
};


#endif /* ASSIGNMENTITEM_H */

