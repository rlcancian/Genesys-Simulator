/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Set.h
 * Author: rlcancian
 *
 * Created on 03 de Junho de 2019, 15:11
 */

#ifndef SET_H
#define SET_H

#include <vector>

#include "kernel/simulator/ModelDataDefinition.h"
#include "kernel/simulator/ModelDataManager.h"
#include "kernel/simulator/ParserChangesInformation.h"
#include "kernel/simulator/PluginInformation.h"
#include "kernel/simulator/EntityType.h"

/*!
 Set module
DESCRIPTION
This data module defines various types of sets, including resource, counter, tally,
entity type, and entity picture. Resource sets can be used in the Process modules (and
Seize, Release, Enter, and Leave of the Advanced Process and Advanced Transfer
panels). Counter and Tally sets can be used in the Record module. Queue sets can be
used with the Seize, Hold, Access, Request, Leave, and Allocate modules of the
Advanced Process and Advanced Transfer panels.
TYPICAL USES
* Machines that can perform the same operations in a manufacturing facility
* Supervisors, check-out clerks in a store
* Shipping clerks, receptionists in an office
* Set of pictures corresponding to a set of entity types
PROMPTS
Prompt Description
Name The unique name of the set being defined.
Type Type of set being defined.
Members Repeat group that specifies the resource members with the set.
The order of listing the members within the repeat group is
important when using selection rules such as Preferred Order and
Cyclical.
Resource Name Name of the resource to include in the resource set. Applies only
when Type is Resource.
Tally Name Name of the tally within the tally set. Applies only when Type is
Tally.
Counter Name Name of the counter within the counter set. Applies only when
Type is Counter.
Entity Type Name of the entity type within the entity type set. Applies only
when Type is Entity.
Picture Name Name of the picture within the picture set. Applies only when
Type is Entity Picture.
 */
class Set : public ModelDataDefinition {
public:
	Set(Model* model, std::string name = "", std::vector<std::string> allowedElementTypes = {});
	virtual ~Set() override;
public: // static
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord *fields);
	static PluginInformation* GetPluginInformation();
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
public:
	virtual std::string show() override;
public:
	//void setSetOfType(std::string _setOfType);
	std::string getSetOfType() const;
	/*! \brief Sets the concrete member type for this Set while it is still structurally compatible.
	 *
	 * A Set is polymorphic only before its first member is inserted.  Once members exist, all
	 * subsequent members must match this selected ModelDataDefinition subclass name.
	 */
	bool setSetOfType(const std::string& setOfType);
	/*! \brief Returns true while the selected concrete member type can still be changed. */
	bool canChangeSetOfType() const;
	/*! \brief Restricts which ModelDataDefinition subclasses may be created through this Set.
	 *
	 * Empty means "use every creatable ModelDataDefinition plugin known by the simulator".
	 * Context owners such as SeizableItem and QueueableItem use this to expose Resource-only or
	 * Queue-only Set editing in the GUI without hard-coding those rules in the editor.
	 */
	void setAllowedElementTypes(const std::vector<std::string>& allowedElementTypes);
	/*! \brief Adds one accepted ModelDataDefinition subclass name to this Set creation contract. */
	void addAllowedElementType(const std::string& allowedElementType);
	/*! \brief Returns the currently accepted concrete member types for GUI and tooling contracts. */
	std::vector<std::string> getAllowedElementTypes() const;
	List<ModelDataDefinition*>* getElementSet() const;

	void addElementSet(ModelDataDefinition* newElement);
	void removeElementSet(ModelDataDefinition* element);
	/*! \brief Creates a new model data definition of the selected Set member type.
	 *
	 * This method is the kernel-side factory used by generic property editors.  The editor provides
	 * the intended type; Set validates the current type/allowed-type contract and delegates actual
	 * object construction to PluginManager so plugin registration rules stay centralized.
	 */
	ModelDataDefinition* createElementSetOfType(const std::string& typeName, const std::string& name = "");

protected: // must be overriden 
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
protected: // could be overriden 
	virtual bool _check(std::string& errorMessage) override;
	virtual ParserChangesInformation* _getParserChangesInformation() override;
    virtual void _createInternalAndAttachedData() override;
private:
	//ElementManager* _elems;

	const struct DEFAULT_VALUES {
		const unsigned int membersSize = 0;
		const std::string setOfType = "";
	} DEFAULT;
	List<ModelDataDefinition*>* _elementSet = new List<ModelDataDefinition*>();
	std::string _setOfType = DEFAULT.setOfType;
	std::vector<std::string> _allowedElementTypes;
};

#endif /* SET_H */
