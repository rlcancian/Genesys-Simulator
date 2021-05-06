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

#include "ModelElement.h"
#include "ElementManager.h"
#include "ParserChangesInformation.h"
#include "PluginInformation.h"
#include "EntityType.h"
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
 Machines that can perform the same operations in a manufacturing facility
 Supervisors, check-out clerks in a store
 Shipping clerks, receptionists in an office
 Set of pictures corresponding to a set of entity types
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
class Set : public ModelElement {
public:
	Set(Model* model, std::string name = "");
	virtual ~Set() = default;
public: // static
	static ModelElement* LoadInstance(Model* model, std::map<std::string, std::string>* fields);
	static PluginInformation* GetPluginInformation();
public:
	virtual std::string show();
public:
	void setSetOfType(std::string _setOfType);
	std::string getSetOfType() const;
	List<ModelElement*>* getElementSet() const;

protected: // must be overriden by derived classes
	virtual bool _loadInstance(std::map<std::string, std::string>* fields);
	virtual std::map<std::string, std::string>* _saveInstance();
protected: // could be overriden by derived classes
	virtual bool _check(std::string* errorMessage);
	virtual ParserChangesInformation* _getParserChangesInformation();
private:
	//ElementManager* _elems;
	const struct DEFAULT_VALUES {
		unsigned int membersSize = 0;
		std::string setOfType = Util::TypeOf<EntityType>();
	} DEFAULT;
	List<ModelElement*>* _elementSet = new List<ModelElement*>();
	std::string _setOfType;
};

#endif /* SET_H */

