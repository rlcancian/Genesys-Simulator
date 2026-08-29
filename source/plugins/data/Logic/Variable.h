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

#include "../../../kernel/simulator/essentialPlugins/Attribute.h"
#include "../../../kernel/simulator/model/ModelDataManager.h"
#include "kernel/simulator/Plugin.h"
#include "../../../kernel/simulator/essentialPlugins/SparseValueStore.h"

/*!
 * \brief Data definition for a named value (scalar or indexed array) that is
 * global to the whole model, as opposed to Attribute's per-entity values.
 *
 * Arena correspondence: the "Variable module" (Rockwell Automation,
 * *Getting Started with Arena*, "The Basic Process Panel", pp. 48-49).
 *
 * GenESyS models Variable as a subclass of Attribute, reusing its
 * dimension/rows/columns model and sparse initial-value store
 * (\c SparseValueStore) instead of duplicating that infrastructure; \c
 * getValue()/setValue() operate on a second, independent runtime store
 * (\c _values) that starts from the inherited initial values but is not
 * per-entity.
 *
 * Known differences from Arena: there is no "Clear Option" (Statistics /
 * System / None reset timing) and no external-file linkage (File Name,
 * Recordset, File Read Time); a Variable's runtime values are only reset
 * through \c _initBetweenReplications().
 */
class Variable : public Attribute {
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
	/*! \brief Copies the full runtime sparse store from another variable. */
	void copyValuesFrom(const Variable& source);
	/*! \brief Returns the initial values serialized in bracket notation. */
	std::string getInitialValuesText() const override;
	/*! \brief Parses bracket notation and replaces both initial and runtime stores. */
	void setInitialValuesText(std::string valuesText) override;
	/*! \brief Appends one dimension size to the variable definition. */
	void insertDimentionSize(unsigned int size);
	/*! \brief Returns mutable current sparse values for compatibility with existing callers. */
	std::map<std::string, double> *getValues() const;
	/*! \brief Returns current value store used by this variable. */
	SparseValueStore* getValueStore();
	ModelDataDefinition* get_scope();
	void set_scope(ModelDataDefinition* const scope);

protected:
    virtual bool _loadInstance(PersistenceRecord *fields) override;
    virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
    virtual bool _check(std::string& errorMessage) override;
    virtual void _initBetweenReplications() override;


protected:
	// virtual void _createInternalStatisticReporters() override;
	// virtual void _createNonEditableDataDefinitions() override;
	// virtual void _createEditableDataDefinitions() override;
	// virtual void _createAttachedAttributes() override;

private:
    //const struct DEFAULT_VALUES {	} DEFAULT;
	SparseValueStore* _values = new SparseValueStore();
	ModelDataDefinition* scope = nullptr; // not used so far
};

#endif /* VARIABLE_H */
