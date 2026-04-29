/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   PickStation.h
 * Author: Prof. Rafael Luiz Cancian, Dr. Eng.
 *
 * Created on 11 de Setembro de 2019, 13:08
 */

#ifndef PICKSTATION_H
#define PICKSTATION_H

#include "kernel/simulator/ModelComponent.h"
#include "kernel/simulator/ModelDataDefinition.h"
#include "kernel/util/List.h"
#include "plugins/data/DiscreteProcessing/Queue.h"
#include "plugins/data/DiscreteProcessing/Resource.h"

#include "plugins/components/Decisions/PickableStationItem.h"

/*!
 PickStation module
DESCRIPTION
The PickStation module allows an entity to select a particular station from the
multiple stations specified. This module picks among the group of stations based on
the selection logic defined with the module. The entity may then route, transport,
convey, or connect to the station specified. If the method chosen is connect, the
selected station is assigned to an entity attribute. The station selection process is
based on the minimum or maximum value of a variety of system variables and
expressions.
TYPICAL USES
 * A part sent to a processing station based on machine’s availability at each station
 * A loan application sent to a set of loan officers based on the number sent to each
officer
 * A customer selecting among cashier lines based on the least number waiting in
each line
PROMPTS
Prompt Description
Name Unique name of the module that will be displayed in the
flowchart.
Test Condition Test condition to use for the station selection process, either
Minimum or Maximum.
Number En Route to
Station
The number of entities transferring to the station is considered in
the station selection process.
Number in Queue The number of entities in the queue at the station is considered in
the station selection process.
Number of Resources
Busy
The number of busy resources at the station is considered in the
station selection process.
Expression Determines if an additional user-defined expression is
considered in the station selection process.
Transfer Type Determines how an entity will be transferred out of this module
to its next destination station—either Route, Convey,
Transport, or Connect.
Save Attribute Defines the name of the attribute that will store the station name
that is selected, visible when the transfer method is Connect.
Route Time Move time of the entity from its current station to the station
determined through this module.
Units Time units for route-time parameters.
 */
class PickStation : public ModelComponent {
public:

	enum class TestCondition : int {
		MINIMUM = 0, MAXIMUM = 1, num_elements = 2
	};

	//enum class PickCondition : int {
	//	NUMBER_BUSY_RESOURCE = 1, NUMBER_IN_QUEUE = 2, EXPRESSION = 3
	//};
public:
	static std::string convertEnumToStr(TestCondition condition);
public: // constructors
	PickStation(Model* model, std::string name = "");
	virtual ~PickStation() = default;
public: // virtual
	virtual std::string show() override;
public:
	//void setPickCondition(PickStation::PickCondition _pickCondition);
	//PickStation::PickCondition getPickCondition() const;
	void setTestCondition(PickStation::TestCondition _testCondition);
	PickStation::TestCondition getTestCondition() const;
	void setSaveAttribute(std::string _saveAttribute);
	std::string getSaveAttribute() const;
	List<PickableStationItem*>* getPickableStationItens() const;
	void addPickableStationItem(PickableStationItem* newItem);
	void removePickableStationItem(PickableStationItem* item);
	void setPickConditionExpression(bool _pickConditionExpression);
	bool isPickConditionExpression() const;
	void setPickConditionNumberInQueue(bool _pickConditionNumberInQueue);
	bool isPickConditionNumberInQueue() const;
	void setPickConditionNumberBusyResource(bool _pickConditionNumberBusyResource);
	bool isPickConditionNumberBusyResource() const;
public: // static
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");


protected: // must be overriden 
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override;
protected: // could be overriden by derived classes
	virtual bool _check(std::string& errorMessage) override;
	/*! This method returns all changes in the parser that are needed by plugins of this ModelDatas. When connecting a new plugin, ParserChangesInformation are used to change parser source code, whch is after compiled and dinamically linked to to simulator kernel to reflect the changes */
	virtual ParserChangesInformation* _getParserChangesInformation() override;
	virtual void _initBetweenReplications() override;
	/*! This method is necessary only for those components that instantiate internal elements that must exist before simulation starts and even before model checking. That's the case of components that have internal StatisticsCollectors, since others components may refer to them as expressions (as in "TVAG(ThisCSTAT)") and therefore the modeldatum must exist before checking such expression */
	virtual void _createInternalAndAttachedData() override; /*< A ModelDataDefinition or ModelComponent that includes (internal) ou refers to (attach) other ModelDataDefinition must register them inside this method. */
	virtual void _addSimulationControl(SimulationControl* property) override;


protected:
	void _doCreateReportStatisticsDataDefinitions();
	void _doCreateEditableDataDefinitions();
	void _doCreateOthersDataDefinitions();


	void _createReportStatisticsDataDefinitions() override;
	void _createEditableDataDefinitions() override;
	void _createOthersDataDefinitions() override;

private: // methods
private: // attributes 1:1

	const struct DEFAULT_VALUES {
		const std::string saveAttribute = "";
		const PickStation::TestCondition testCondition = TestCondition::MINIMUM;
		bool pickConditionNumberBusyResource = false;
		bool pickConditionNumberInQueue = false;
		bool pickConditionExpression = true;
		//const PickStation::PickCondition pickCondition = PickCondition::NUMBER_BUSY_RESOURCE;
	} DEFAULT;
	std::string _saveAttribute = DEFAULT.saveAttribute;
	PickStation::TestCondition _testCondition = DEFAULT.testCondition;
	//PickStation::PickCondition _pickCondition = DEFAULT.pickCondition;
	bool _pickConditionNumberBusyResource = DEFAULT.pickConditionNumberBusyResource;
	bool _pickConditionNumberInQueue = DEFAULT.pickConditionNumberInQueue;
	bool _pickConditionExpression = DEFAULT.pickConditionExpression;

	//DummyElement* _internalDataDefinition = nullptr;
private: // attributes 1:n
	List<PickableStationItem*>* _pickableStationItens = new List<PickableStationItem*>();
};


#endif /* PICKSTATION_H */
