/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   SeizableItem.h
 * Author: rlcancian
 *
 * Created on 10 de abril de 2021, 08:45
 */

#ifndef SEIZABLEITEM_H
#define SEIZABLEITEM_H

#include "plugins/data/DiscreteProcessing/Resource.h"
#include "plugins/data/DiscreteProcessing/Set.h"
#include "kernel/simulator/ModelDataManager.h"

// @TODO should inhere from a common base to QueueableItem

class SeizableItem {
public:
	enum class SelectionRule : int {
		CYCLICAL = 0, RANDOM = 1, SPECIFICMEMBER = 2, LARGESTREMAININGCAPACITY = 3, SMALLESTNUMBERBUSY = 4,
		PREFEREDORDER = 5, num_elements = 6
	};

	enum class SeizableType : int {
		RESOURCE = 0, SET = 1, num_elements = 2
	};

	static std::string convertEnumToStr(SelectionRule rule);
	static std::string convertEnumToStr(SeizableType type);

public:
	SeizableItem(ModelDataDefinition* resourceOrSet, std::string quantityExpression = "1",
	             SeizableItem::SelectionRule selectionRule = SeizableItem::SelectionRule::LARGESTREMAININGCAPACITY,
	             std::string saveAttribute = "", std::string index = "0");
	SeizableItem(Model* model, std::string resourceName, std::string quantityExpression = "1",
	             SeizableItem::SelectionRule selectionRule = SeizableItem::SelectionRule::LARGESTREMAININGCAPACITY,
	             std::string saveAttribute = "", std::string index = "0");
	SeizableItem(SeizableItem* original);

public:
	bool loadInstance(PersistenceRecord* fields, unsigned int parentIndex);
	void saveInstance(PersistenceRecord* fields, unsigned int parentIndex, bool saveDefault);
	bool loadInstance(PersistenceRecord* fields);
	void saveInstance(PersistenceRecord* fields, bool saveDefaults);

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
	std::string getName() const;
	void setResource(Resource* resource);
	Resource* getResource() const;
	void setSet(Set* set);
	Set* getSet() const;
	//void setSeizableType(SeizableType resourceType);
	SeizableType getSeizableType() const;
	void setLastMemberSeized(unsigned int lastMemberSeized);
	unsigned int getLastMemberSeized() const;
	ModelDataDefinition* getSeizable() const;
	void setElementManager(ModelDataManager* modeldataManager);
	void setLastPreferedOrder(unsigned int lastPreferedOrder);
	unsigned int getLastPreferedOrder() const;
	//void setComponentManager(ComponentManager* _componentManager);
	List<SimulationControl*>* getSimulationControls() const;
	void _addSimulationControl(SimulationControl* control);

	std::string getTypeDC() { return _typeDC; };

private:
	void _ensureSimulationControls(Model* model);

private:
	const struct DEFAULT_VALUES {
		const std::string quantityExpression = "1";
		const SeizableItem::SeizableType seizableType = SeizableItem::SeizableType::RESOURCE;
		const SeizableItem::SelectionRule selectionRule = SeizableItem::SelectionRule::LARGESTREMAININGCAPACITY;
		const std::string saveAttribute = "";
		const std::string index = "0";
	} DEFAULT;

	SeizableType _seizableType = DEFAULT.seizableType;
	SelectionRule _selectionRule = DEFAULT.selectionRule;
	std::string _saveAttribute = DEFAULT.saveAttribute;
	std::string _index = DEFAULT.index;
	ModelDataDefinition* _resourceOrSet = nullptr;
	std::string _seizableName;
	std::string _quantityExpression;
	unsigned int _lastMemberSeized = 0;
	unsigned int _lastPreferedOrder = 0;
	std::string _typeDC = Util::TypeOf<Resource>();

private:
	ModelDataManager* _modeldataManager;
	List<SimulationControl*>* _simulationControls = new List<SimulationControl*>();
};

#endif /* SEIZABLEITEM_H */
