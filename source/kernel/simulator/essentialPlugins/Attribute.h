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
#include "../model/ModelDataDefinition.h"
#include "../model/ModelDataManager.h"
#include "../PluginInformation.h"
#include "SparseValueStore.h"

//namespace GenesysKernel {

/*!
 * \brief Data definition declaring one named, per-entity attribute (its
 * dimensionality and sparse initial values), shared across every entity
 * created in the model.
 *
 * Arena correspondence: the "Attribute module" (Rockwell Automation,
 * *Getting Started with Arena*, "The Basic Process Panel", p. 43), which
 * defines an attribute's rows/columns, data type and initial value(s).
 * Attribute values are per-entity (each Entity instance holds its own
 * store), which is the same distinction Arena makes between per-entity
 * Attributes and global Variables — see Variable, which extends this class
 * to add global (model-wide) storage.
 *
 * Initial values are kept as a sparse, indexed store (SparseValueStore)
 * addressed by a scalar or textual index, and can also be edited through the
 * bracket-notation text form (\c getInitialValuesText()/setInitialValuesText()).
 *
 * Known difference from Arena: only a numeric (\c double) value type is
 * modeled; Arena's "String" data type has no equivalent here.
 */
class Attribute : public ModelDataDefinition {
public:
	/*! \brief Creates an entity attribute definition in the model. */
	Attribute(Model* model, std::string name = "", std::string dataDefinitionTypename = Util::TypeOf<Attribute>());
	/*! \brief Releases sparse initial values owned by this definition. */
	virtual ~Attribute() override;
public:
	virtual std::string show() override;
public: // public static methods
	static PluginInformation* GetPluginInformation();
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
public:
	/*! \brief Returns the initial values serialized in bracket notation. */
	virtual std::string getInitialValuesText() const;
	/*! \brief Parses bracket notation and replaces the initial value store. */
	virtual void setInitialValuesText(std::string valuesText);
	/*! \brief Reads the initial sparse value, returning 0.0 when the index is absent. */
	double getInitialValue(std::string index = "");
	/*! \brief Writes the initial sparse value at the scalar or indexed position. */
	void setInitialValue(double value, std::string index = "");
	/*! \brief Replaces/extends initial sparse values from textual index/value pairs. */
	void setInitialValues(const std::vector<std::pair<std::string,double>> values);
	/*! \brief Copies the full initial sparse store from another attribute. */
	void copyInitialValuesFrom(const Attribute& source);
	/*! \brief Appends one dimension size to the attribute definition. */
	void insertDimentionSize(unsigned int size);
	/*! \brief Returns dimension sizes for compatibility with existing callers. */
	std::list<unsigned int>* getDimensionSizes() const;
	/*! \brief Returns mutable initial sparse values for compatibility with existing callers. */
	std::map<std::string, double>* getInitialValues() const;
	/*! \brief Returns the sparse initial value store used by this attribute. */
	SparseValueStore* getInitialValueStore();
	/*! \brief Returns whether the last textual initial value is syntactically valid. */
	bool isInitialValuesTextValid() const;
protected: //! must be overriden by derived classes
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
protected: //! could be overriden by derived classes
	virtual bool _check(std::string& errorMessage) override;
private:
	void _syncInitialValuesTextFromStore();
	SparseValueStore* _initialValues = new SparseValueStore();
	std::string _initialValuesText = "0";
	bool _initialValuesTextValid = true;
	std::string _initialValuesTextErrorMessage;
};
//namespace\\}
#endif /* ATTRIBUTE_H */
