/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   Variable.h
 * Author: rafael.luiz.cancian
 *
 * Created on 4 de Setembro de 2018, 18:28
 */

#ifndef VARIABLE_H
#define VARIABLE_H

#include "kernel/simulator/ModelDataDefinition.h"
#include "kernel/simulator/ModelDataManager.h"
#include "kernel/simulator/Plugin.h"
#include "kernel/simulator/SparseValueStore.h"

/*!
Variable module
DESCRIPTION
This data module is used to define a variable’s dimension and values. You can
reference variables in other modules (for example, the Decide module), reassign new
values to variables with the Assign module, and use variables in any expression.
You can use an external data file to specify variable values, and you can specify the
variable’s initial values in the Variable module. If you use both methods, the values
are read at different times, depending on the options you specify, including the File
Read Time, the Clear Option, and the replication parameters you specify in the Run
Setup dialog box. For more information, see the online Help.
There are three methods for manually editing the Initial Values of a Variable module:
* Using the standard spreadsheet interface. In the module spreadsheet, right-click
on the Initial Values cell and select the Edit via spreadsheet menu item. The
values for two-dimensional arrays should be entered one column at a time. Array
elements not explicitly assigned are assumed to have the last entered value.
* Using the module dialog box. In the module spreadsheet, right-click on any cell
and select the Edit via dialog menu item. The values for two-dimensional arrays
should be entered one column at a time. Array elements not explicitly assigned
are assumed to have the last entered value.
* Using the two-dimensional (2-D) spreadsheet interface. In the module
spreadsheet, click on the Initial Values cell.
TYPICAL USES
* Number of documents processed per hour
* Serial number to assign to parts for unique identification
* Space available in a facility
PROMPTS
Prompt Description
Name The unique name of the variable being defined.
Rows Number of rows in a one- or two-dimensional variable.
Columns Number of columns in a two-dimensional variable.
Report Statistics Check box for determining whether or not statistics will be
collected. This field is visible when the rows and columns are
not specified (that is, for single variables).
Data Type The data type of the values stored in the variable. Valid types are
Real and String. The default type is Real.
Clear Option Defines the time (if at all) when the value(s) of the variable is
reset to the initial value(s) specified. Specifying Statistics resets
this variable to its initial value(s) whenever statistics are cleared.
Specifying System resets this variable to its initial value(s)
whenever the system is cleared. Specifying None indicates that
this variable is never reset to its initial value(s), except prior to
the first replication.
File Name Name of the file from which to read the variable's value or
values. You can use any file access type supported by Arena
except sequential text files and Lotus spreadsheet (.wks) files. If
the file name you specify has not been created yet, Arena will
create it, but you must edit the file to specify the file access type,
path, and recordset (if required).
Recordset Name of the recordset in the specified file from which to read
values. This field is available only if you specify a File Name for
a file that has been set up with a file access type, path, and
recordset. Arena uses the Rows and Columns properties to
determine the amount of data to read from the recordset. A
recordset is required for all file types except .xml. The recordset
size must be equal to or greater than the number of rows and
columns specified for the variable.
File Read Time Specifies when to read the values from the file into the variable.
If you select PreCheck, the values for the variable are read while
the model is still in Edit mode (prior to the model being checked
and compiled). If you select BeginSimulation, values are read
when the model is compiled, prior to the first replication. If you
select BeginReplication, values are read prior to each
replication.
Initial Values Lists the initial value or values of the variable. You can assign
new values to the variable at different stages of the model by
using the Assign module.
Initial Value Variable value at the start of the simulation.
 */
class Variable : public ModelDataDefinition {
public:
    Variable(Model* model, std::string name = "");
    virtual ~Variable() override;
public:
    virtual std::string show() override;
public: //static
    static PluginInformation* GetPluginInformation();
    static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord *fields);
    static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
public:
	/*! \brief Reads the current sparse value, returning 0.0 when the index is absent. */
	double getValue(std::string index="");
	/*! \brief Writes the current sparse value at the scalar or indexed position. */
	void setValue(double value, std::string index="");
	/*! \brief Reads the initial sparse value, returning 0.0 when the index is absent. */
	double getInitialValue(std::string index = "");
	/*! \brief Writes the initial sparse value at the scalar or indexed position. */
	void setInitialValue(double value, std::string index="");
	/*! \brief Replaces/extends initial sparse values from textual index/value pairs. */
	void setInitialValues(const std::vector<std::pair<std::string,double>> values);
	/*! \brief Appends one dimension size to the variable definition. */
	void insertDimentionSize(unsigned int size);
	/*! \brief Returns dimension sizes for compatibility with existing callers. */
	std::list<unsigned int>* getDimensionSizes() const;
	/*! \brief Returns mutable current sparse values for compatibility with existing callers. */
	std::map<std::string, double> *getValues() const;
	/*! \brief Returns current value store used by this variable. */
	SparseValueStore* getValueStore();
	/*! \brief Returns initial value store used by this variable. */
	SparseValueStore* getInitialValueStore();
	ModelDataDefinition* get_scope();
	void set_scope(ModelDataDefinition* const scope);

	//double getValue();
	//void setValue(double value);
	//void setValue(std::string index, double value);
	//double getInitialValue();
	//void setInitialValue(double value);

protected:
    virtual bool _loadInstance(PersistenceRecord *fields) override;
    virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
    virtual bool _check(std::string& errorMessage) override;
    virtual void _initBetweenReplications() override;

private:
    //const struct DEFAULT_VALUES {	} DEFAULT;
	SparseValueStore* _values = new SparseValueStore();
	SparseValueStore* _initialValues = new SparseValueStore();
	ModelDataDefinition* scope = nullptr; // not used so far
};

#endif /* VARIABLE_H */
