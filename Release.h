/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Release.h
 * Author: rafael.luiz.cancian
 *
 * Created on 21 de Agosto de 2018, 16:17
 */

#ifndef RELEASE_H
#define RELEASE_H

#include <string>

#include "ModelComponent.h"
#include "Resource.h"
#include "Plugin.h"
#include "SeizableItemRequest.h"

/*!
Release module
DESCRIPTION
The Release module is used to release units of a resource that an entity previously has
seized. This module may be used to release individual resources or may be used to
release resources within a set. For each resource to be released, the name and quantity
to release are specified.
When the entity enters the Release module, it gives up control of the specified
resource(s). Any entities waiting in queues for those resources will gain control of the
resources immediately.
TYPICAL USES
 Finishing a customer order (release the operator)
 Completing a tax return (release the accountant)
 Leaving the hospital (release the doctor, nurse, hospital room)
PROMPTS
Prompt Description
Name Unique module identifier displayed on the module shape.
Type Type of resource for releasing, either specifying a particular
resource, or selecting from a pool of resources (that is, a resource
set). The resource name may also be specified by an expression
or attribute value.
Resource Name Name of the resource that will be released.
Set Name Name of the resource set from which a member will be released.
Attribute Name Name of the attribute that specifies the resource name to be
released.
Expression Name of the expression that specifies the name of the resource to
be released.
Quantity Number of resources of a given name or from a given set that
will be released. For sets, this value specifies only the number of
a selected resource that will be released (based on the resource’s
capacity), not the number of members to be released within the
set.
Release Rule Method of determining which resource within a set to release.
Last Member Seized and First Member Seized will release the
last/first member from within the set that was seized. Specific
member indicates that a member number or attribute (with a
member number value) will be used to specify the member to
release.
Set Index Member index of the resource set that the entity will release.  
 */
class Release : public ModelComponent {
public:
	Release(Model* model, std::string name = "");
	virtual ~Release() = default;
public:
	virtual std::string show();
public: //static
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, std::map<std::string, std::string>* fields);
public: // get & set
	void setPriority(unsigned short _priority);
	unsigned short priority() const;
public: // gets
	List<SeizableItemRequest*>* getReleaseRequests() const;

protected:
	virtual void _execute(Entity* entity);
	virtual bool _loadInstance(std::map<std::string, std::string>* fields);
	virtual void _initBetweenReplications();
	virtual std::map<std::string, std::string>* _saveInstance();
	virtual bool _check(std::string* errorMessage);
private:

	const struct DEFAULT_VALUES {
		unsigned short priority = 0;
		unsigned int releaseRequestSize = 1;
	} DEFAULT;
	unsigned short _priority = DEFAULT.priority;
	List<SeizableItemRequest*>* _releaseRequests = new List<SeizableItemRequest*>();
};

#endif /* RELEASE_H */

