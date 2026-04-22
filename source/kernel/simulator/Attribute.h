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
#include <map>
#include <utility>
#include <vector>
#include "ModelDataDefinition.h"
#include "ModelDataManager.h"
#include "PluginInformation.h"
#include "SparseValueStore.h"

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
	/*! \brief Creates an entity attribute definition in the model. */
	Attribute(Model* model, std::string name = "");
	/*! \brief Releases sparse initial values owned by this definition. */
	virtual ~Attribute() override;
public:
	virtual std::string show() override;
public: // public static methods
	static PluginInformation* GetPluginInformation();
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
public:
	/*! \brief Reads the initial sparse value, returning 0.0 when the index is absent. */
	double getInitialValue(std::string index = "");
	/*! \brief Writes the initial sparse value at the scalar or indexed position. */
	void setInitialValue(double value, std::string index = "");
	/*! \brief Replaces/extends initial sparse values from textual index/value pairs. */
	void setInitialValues(const std::vector<std::pair<std::string,double>> values);
	/*! \brief Appends one dimension size to the attribute definition. */
	void insertDimentionSize(unsigned int size);
	/*! \brief Returns dimension sizes for compatibility with existing callers. */
	std::list<unsigned int>* getDimensionSizes() const;
	/*! \brief Returns mutable initial sparse values for compatibility with existing callers. */
	std::map<std::string, double>* getInitialValues() const;
	/*! \brief Returns the sparse initial value store used by this attribute. */
	SparseValueStore* getInitialValueStore();
protected: //! must be overriden by derived classes
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
protected: //! could be overriden by derived classes
	virtual bool _check(std::string& errorMessage) override;
private:
	SparseValueStore* _initialValues = new SparseValueStore();
};
//namespace\\}
#endif /* ATTRIBUTE_H */
