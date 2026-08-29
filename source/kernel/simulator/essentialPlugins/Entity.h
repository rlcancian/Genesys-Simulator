/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Entity.h
 * Author: rafael.luiz.cancian
 *
 * Created on 21 de Junho de 2018, 16:30
 */

#ifndef ENTITY_H
#define ENTITY_H

#include <string>
#include <map>

#include "../../util/Util.h"
#include "../../util/List.h"
#include "../model/ModelDataDefinition.h"
#include "EntityType.h"
#include "SparseValueStore.h"
//namespace GenesysKernel {

/*!
 * \brief Runtime entity (token) flowing through the model during simulation.
 *
 * This is the live, per-replication object created and destroyed by \c Model
 * for each simulated item (a "job", "customer", "part", etc.). It owns the
 * sparse per-entity attribute values (see Attribute) and a pointer to its
 * EntityType, but it is not itself the data definition an end user edits
 * before running the model.
 *
 * Arena correspondence: the configurable "Entity module" (name, initial
 * picture, holding cost/hour and initial VA/NVA/waiting/transfer/other costs;
 * Rockwell Automation, *Getting Started with Arena*, "The Basic Process
 * Panel", pp. 44-45) is a data definition and corresponds to EntityType, not
 * to this class. This class is the closest correspondence to what Arena
 * calls an "entity" at simulation time: an instance created from an entity
 * type, carrying attribute values and a runtime identification number.
 *
 * Construction/destruction are restricted to \c Model so that entity
 * lifetime always stays under simulator control.
 */
class Entity : public ModelDataDefinition {
private: // no one can create or destry entities directlly. This can be done one throught friend class Model
	/*! \brief Creates an entity instance (restricted to \c Model friend). */
	Entity(Model* model, std::string name = "", bool insertIntoModel = true);
	virtual ~Entity();
	// friend Entity* Model::createEntity(std::string name, bool insertIntoModel); // It would be better, but Model is not known at this point of compilaton
	friend class Model;
public:
	virtual std::string show() override;

public: // g & s
	/*!
	 * \brief setEntityTypeName
	 * \param entityTypeName
	 * \details Sets entity type by name (indirect lookup through model data manager).
	 */
	void setEntityTypeName(std::string entityTypeName); //*!< indirect access to EntityType
	/*!
	 * \brief getEntityTypeName
	 * \return Name of the associated entity type.
	 */
	std::string getEntityTypeName() const;
	/*!
	 * \brief setEntityType
	 * \param entityType
	 * \details Directly assigns the entity type pointer.
	 */
	void setEntityType(EntityType* entityType); //*!< direct access to EntityType
	/*!
	 * \brief getEntityType
	 * \return Pointer to the associated entity type.
	 */
	EntityType* getEntityType() const;
public:
	/*!
	 * \brief getAttributeValue
	 * \param index
	 * \param attributeName
	 * \return Current attribute value for the provided name/index.
	 */
	double getAttributeValue(std::string attributeName, std::string index="");
	/*!
	 * \brief getAttributeValue
	 * \param index
	 * \param attributeID
	 * \return Current attribute value for the provided id/index.
	 */
	double getAttributeValue(Util::identification attributeID, std::string index="");
	/*!
	 * \brief setAttributeValue
	 * \param index
	 * \param attributeName
	 * \param value
	 * \param createIfNotFound
	 * \details Assigns an attribute value by name and optionally creates missing attributes.
	 */
	void setAttributeValue(std::string attributeName, double value, std::string index="", bool createIfNotFound=false);
	/*!
	 * \brief setAttributeValue
	 * \param index
	 * \param attributeID
	 * \param value
	 * \details Assigns an attribute value by attribute identifier.
	 */
	void setAttributeValue(Util::identification attributeID, double value, std::string index="");
	/*! \brief Copies all sparse values from one attribute to another for this entity. */
	void copyAttributeValues(std::string destinationAttributeName, std::string sourceAttributeName);
	/*!
	 * \brief entityNumber
	 * \return Unique runtime identifier for this entity instance.
	 */
	Util::identification entityNumber() const;
protected:
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
	virtual bool _check(std::string& errorMessage) override;
private:
	/*! \brief Ensures the sparse store for an attribute rank exists and returns it. */
	SparseValueStore* _ensureAttributeStore(unsigned int rank);

	Util::identification _entityNumber;
	EntityType* _entityType = nullptr;
	List<SparseValueStore*>* _attributeValues = new List<SparseValueStore*>();
};
//namespace\\}
#endif /* ENTITY_H */
