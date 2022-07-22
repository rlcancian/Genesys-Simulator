/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Attribute.h
 * Author: rafael.luiz.cancian
 *
 * Created on 25 de Setembro de 2018, 16:37
 */

#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

#include <string>
#include <list>
#include "../util/List.h"
#include "ModelDataDefinition.h"
#include "ModelDataManager.h"
#include "Plugin.h"

//namespace GenesysKernel {

/*!
 Attribute module
DESCRIPTION
This data module is used to define an attribute’s dimension, data type and initial
value(s). An attribute is a characteristic of all entities created, but with a specific
value that can differ from one entity to another. Attributes can be referenced in other
modules (for example, the Decide module), can be reassigned a new value with the
Assign module, and can be used in any expression. Attribute values are unique for
each entity, as compared to Variables which are global to the simulation module.
There are three methods for manually editing the Initial Values of an Attribute
module:
 Using the standard spreadsheet interface. In the module spreadsheet, rightclick on the Initial Values cell and select the Edit via spreadsheet menu
item. The values for two-dimensional arrays should be entered one column at
a time. Array elements not explicitly assigned are assumed to have the last
entered value.
 Using the module dialog box. In the module spreadsheet, right-click on any
cell and select the Edit via dialog menu item. The values for two-dimensional
arrays should be entered one column at a time. Array elements not explicitly
assigned are assumed to have the last entered value.
 Using the two-dimensional (2-D) spreadsheet interface. In the module
spreadsheet, click on the Initial Values cell.
TYPICAL USES
* Due date of an order (entity)
* Priority of an order (entity)
* Color of a part (entity)
 PROMPTS
 Prompt Description
Name The unique name of the attribute being defined.
Rows Number of rows in a one- or two-dimensional attribute.
Columns Number of columns in a two-dimensional attribute.
Data Type The data type of the values stored in the attribute. Valid types are
Real and String. The default type is Real.
Initial Values Lists the initial value or values of the attribute. You can assign
new values to the attribute by using the Assign module.
Initial Value Entity attribute value when entity is created and enters the
system.
 */
class Attribute : public ModelDataDefinition {
public:
	Attribute(Model* model, std::string name = "");
	virtual ~Attribute() = default;
public:
	virtual std::string show();
public: //static
	static PluginInformation* GetPluginInformation();
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
protected:
	virtual bool _loadInstance(PersistenceRecord *fields);
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues);
	virtual bool _check(std::string* errorMessage);
private:
	//List<unsigned int>* _dimensionSizes = new List<unsigned int>();
};
//namespace\\}
#endif /* ATTRIBUTE_H */

