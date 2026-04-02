/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ModelPersistence_if.h
 * Author: rafael.luiz.cancian
 *
 * Created on 24 de Agosto de 2018, 19:22
 */

#ifndef MODELPERSISTENCE_IF_H
#define MODELPERSISTENCE_IF_H

#include <string>
#include <map>


// forward decl
class PersistenceRecord;

/*!
 * \brief Interface for model serialization/deserialization services.
 *
 * Persistence implementations convert in-memory model structures to/from external
 * representations (e.g., GEN/XML/JSON-like formats) and expose formatting/options
 * hooks used by the kernel persistence pipeline.
 */
class ModelPersistence_if {
public:

	enum class Options : int {
		SAVEDEFAULTS = 1, HIDEIDKEY = 2, HIDETYPEKEY = 4, HIDENAMEKEY = 8, SORTALPHLY = 16
	};

public:
	/*! \brief Serializes and saves the model to the provided filename. */
	virtual bool save(std::string filename) = 0;
	/*! \brief Loads and deserializes a model from file. */
	virtual bool load(std::string filename) = 0;

public:
	/*! \brief Indicates whether persistent state changed since the last save/load. */
	virtual bool hasChanged() = 0;
	/*! \brief Returns whether a persistence option is currently enabled. */
	virtual bool getOption(ModelPersistence_if::Options option) = 0;
	/*! \brief Enables or disables a persistence option. */
	virtual void setOption(ModelPersistence_if::Options option, bool value) = 0;
public:
	/*! \brief Formats a set of fields as text according to the persistence format. */
	virtual std::string getFormatedField(PersistenceRecord *fields) = 0;
};

#endif /* MODELPERSISTENCE_IF_H */

