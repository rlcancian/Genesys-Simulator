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

#include "Util.h"
#include "List.h"
#include "ModelElement.h"
#include "EntityType.h"
//namespace GenesysKernel {

	/*!
	 Entity module
	DESCRIPTION
	This data module defines the various entity types and their initial picture values in a
	simulation. Initial costing information and holding costs are also defined for the
	entity.
	TYPICAL USES
	 Items being produced or assembled (parts, pallets)
	 Documents (forms, e-mails, faxes, reports)
	 People moving through a process (customers, callers)
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
	class Entity : public ModelElement {
	public:
		Entity(Model* model);
		virtual ~Entity() = default;
	public:
		virtual std::string show();

	public: // g & s
		void setEntityTypeName(std::string entityTypeName) throw (); // indirect access to EntityType
		std::string getEntityTypeName() const;
		void setEntityType(EntityType* entityType); // direct access to EntityType
		EntityType* getEntityType() const;
	public:
		double attributeValue(std::string attributeName);
		double attributeValue(std::string index, std::string attributeName);
		double attributeValue(Util::identification attributeID);
		double attributeValue(std::string index, Util::identification attributeID);
		void setAttributeValue(std::string attributeName, double value);
		void setAttributeValue(std::string index, std::string attributeName, double value);
		void setAttributeValue(Util::identification attributeID, double value);
		void setAttributeValue(std::string index, Util::identification attributeID, double value);
		Util::identification entityNumber() const;
	protected:
		virtual bool _loadInstance(std::map<std::string, std::string>* fields);
		virtual std::map<std::string, std::string>* _saveInstance();
		virtual bool _check(std::string* errorMessage);
	private:
		Util::identification _entityNumber;
		EntityType* _entityType = nullptr;
		List< std::map<std::string, double>* >* _attributeValues = new List<std::map<std::string, double>*>();
	};
//namespace\\}
#endif /* ENTITY_H */

