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

#include "../util/Util.h"
#include "../util/List.h"
#include "ModelDataDefinition.h"
#include "EntityType.h"
//namespace GenesysKernel {

/*!
 Entity module
DESCRIPTION
This data module defines the various entity types and their initial picture values in a
simulation. Initial costing information and holding costs are also defined for the
entity.
TYPICAL USES
* Items being produced or assembled (parts, pallets)
* Documents (forms, e-mails, faxes, reports)
* People moving through a process (customers, callers)
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
Prompt Description
Entity Type The name of the entity type being defined. This name must be
unique.
Initial Picture Graphical representation of the entity at the start of the
simulation. This value can be changed during the simulation
using the Assign module.
Holding Cost/Hour Hourly cost of processing the entity through the system. This
cost is incurred when the entity is anywhere in the system.
Initial VA Cost Initial cost value that will be assigned to the value-added cost
attribute of the entity. This attribute accrues the costs incurred
when an entity is spending time in a value-added activity.
Initial NVA Cost Initial cost value that will be assigned to the non-value-added
cost attribute of the entity. This attribute accrues the costs
incurred when an entity is spending time in a non-value-added
activity.
Initial Waiting Cost Initial cost value that will be assigned to the waiting-cost
attribute of the entity. This attribute accrues the costs incurred
when an entity is spending time in a wait activity; for example,
waiting to be batched or waiting for resource(s) at a Process
module.
Initial Transfer Cost Initial cost value that will be assigned to the transfer cost
attribute of the entity. This attribute accrues the costs incurred
when an entity is spending time in a transfer activity.
Initial Other Cost Initial cost value that will be assigned to the other cost attribute
of the entity. This attribute accrues the costs incurred when an
entity is spending time in another activity.
Report Statistics Specifies whether or not statistics will be collected automatically
and stored in the report database for this entity type.
 */
class Entity : public ModelDataDefinition {
private: // no one can create or destry entities directlly. This can be done one throught friend class Model
	Entity(Model* model, std::string name = "", bool insertIntoModel = true);
	virtual ~Entity() = default;
	// friend Entity* Model::createEntity(std::string name, bool insertIntoModel); // It would be better, but Model is not known at this point of compilaton
	friend class Model;
public:
	virtual std::string show() override;

public: // g & s
	/*!
	 * \brief setEntityTypeName
	 * \param entityTypeName
	 */
	void setEntityTypeName(std::string entityTypeName); //*!< indirect access to EntityType
	/*!
	 * \brief getEntityTypeName
	 * \return
	 */
	std::string getEntityTypeName() const;
	/*!
	 * \brief setEntityType
	 * \param entityType
	 */
	void setEntityType(EntityType* entityType); //*!< direct access to EntityType
	/*!
	 * \brief getEntityType
	 * \return
	 */
	EntityType* getEntityType() const;
public:
	/*!
	 * \brief getAttributeValue
	 * \param index
	 * \param attributeName
	 * \return
	 */
	double getAttributeValue(std::string attributeName, std::string index="");
	/*!
	 * \brief getAttributeValue
	 * \param index
	 * \param attributeID
	 * \return
	 */
	double getAttributeValue(Util::identification attributeID, std::string index="");
	/*!
	 * \brief setAttributeValue
	 * \param index
	 * \param attributeName
	 * \param value
	 * \param createIfNotFound
	 */
	void setAttributeValue(std::string attributeName, double value, std::string index="", bool createIfNotFound=false);
	/*!
	 * \brief setAttributeValue
	 * \param index
	 * \param attributeID
	 * \param value
	 */
	void setAttributeValue(Util::identification attributeID, double value, std::string index="");
	/*!
	 * \brief entityNumber
	 * \return
	 */
	Util::identification entityNumber() const;
protected:
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
	virtual bool _check(std::string& errorMessage) override;
private:
	Util::identification _entityNumber;
	EntityType* _entityType = nullptr;
	List< std::map<std::string, double>* >* _attributeValues = new List<std::map<std::string, double>*>();
};
//namespace\\}
#endif /* ENTITY_H */

