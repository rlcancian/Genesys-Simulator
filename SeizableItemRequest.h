/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   SeizableItemRequest.h
 * Author: rlcancian
 *
 * Created on 10 de abril de 2021, 08:45
 */

#ifndef SEIZABLEITEMREQUEST_H
#define SEIZABLEITEMREQUEST_H

#include "Resource.h"
#include "Set.h"
#include "ComponentManager.h"

// \todo should inhere from a common base to QueueableItemRequest
class SeizableItemRequest  {
public:

	enum class SelectionRule : int {
		CYCLICAL = 1, RANDOM = 2, SPECIFICMEMBER = 3, LARGESTREMAININGCAPACITY = 4, SMALLESTNUMBERBUSY = 5
	};

	enum class SeizableType : int {
		RESOURCE = 1, SET = 2
	};

public:
	SeizableItemRequest(ModelElement* resourceOrSet, std::string quantityExpression = "1", SeizableItemRequest::SeizableType resourceType = SeizableItemRequest::SeizableType::RESOURCE, SeizableItemRequest::SelectionRule selectionRule = SeizableItemRequest::SelectionRule::LARGESTREMAININGCAPACITY, std::string saveAttribute = "", std::string index = "0");

public: 
	bool loadInstance(std::map<std::string, std::string>* fields, unsigned int parentIndex);
	std::map<std::string, std::string>* saveInstance(unsigned int parentIndex);
	bool loadInstance(std::map<std::string, std::string>* fields);
	std::map<std::string, std::string>* saveInstance();
public:
	std::string show();
	void setIndex(std::string index);
	std::string getIndex() const;
	void setSaveAttribute(std::string saveAttribute);
	std::string getSaveAttribute() const;
	void setSelectionRule(SelectionRule selectionRule);
	SelectionRule getSelectionRule() const;
	void setQuantityExpression(std::string quantityExpression);
	std::string getQuantityExpression() const;
	std::string getResourceName() const;
	void setResource(Resource* resource);
	Resource* getResource() const;
	void setSet(Set* set);
	Set* getSet() const;
	void setSeizableType(SeizableType resourceType);
	SeizableType getSeizableType() const;
	void setLastMemberSeized(unsigned int lastMemberSeized);
	unsigned int getLastMemberSeized() const;
    ModelElement* getSeizable() const;
    void setComponentManager(ComponentManager* _componentManager);

private:

	const struct DEFAULT_VALUES {
		const std::string quantityExpression = "1";
		const SeizableItemRequest::SeizableType seizableType = SeizableItemRequest::SeizableType::RESOURCE;
		const SeizableItemRequest::SelectionRule selectionRule = SeizableItemRequest::SelectionRule::LARGESTREMAININGCAPACITY;
		const std::string saveAttribute = "";
		const std::string index = "0";
	} DEFAULT;

	SeizableType _seizableType;
	ModelElement* _resourceOrSet;
	std::string _seizableName;
	std::string _quantityExpression;
	SelectionRule _selectionRule;
	std::string _saveAttribute;
	std::string _index;
	unsigned int _lastMemberSeized = 0;
private:
	ComponentManager* _componentManager;
};

#endif /* SEIZABLEITEMREQUEST_H */

