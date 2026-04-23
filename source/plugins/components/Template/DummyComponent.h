/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Dummy.h
 * Author: Prof. Rafael Luiz Cancian, Dr. Eng.
 *
 * Created on 22 de Maio de 2019, 18:41
 */

#pragma once

#include "kernel/simulator/ModelComponent.h"
#include "plugins/data/Template/DummyElement.h"

/*!
 This component ...
 */
class DummyComponent : public ModelComponent {
public: /// constructors
	DummyComponent(Model* model, std::string name = "");
	virtual ~DummyComponent() = default;

public: /// new public user methods for this component
	// ...

public: /// virtual public methods
	virtual std::string show() override;

public: /// static public methods that must have implementations (Load and New just the same. GetInformation must provide specific infos for the new component
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

protected: /// virtual protected method that must be overriden
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override; ///< This method is only for ModelComponents, not ModelDataElements


protected: /// virtual protected methods that could be overriden by derived classes, if needed
	/*! This method is called by ModelChecker during model check. The component should check itself to verify if user parameters are ok (ex: correct syntax for the parser) and everithing in its parameters allow the model too run without errors in this component */
	// virtual bool _check(std::string& errorMessage);
	/*! This method returns all changes in the parser that are needed by plugins of this ModelDatas. When connecting a new plugin, ParserChangesInformation are used to change parser source code, whch is after compiled and dinamically linked to to simulator kernel to reflect the changes */
	// virtual ParserChangesInformation* _getParserChangesInformation();
	/*! This method is called by ModelSimulation when initianting the replication. The model should set all value for a new replication (Ex: setting back to 0 any internal counter, clearing lists, etc. */
	// virtual void _initBetweenReplications();
	void _createReportStatisticsDataDefinitions() override;
	void _createEditableDataDefinitions() override;
	void _createOthersDataDefinitions() override;
	/*! This method is called by ModelChecker and is necessary only for those components that instantiate internal elements that must exist before simulation starts and even before model checking. That's the case of components that have internal StatisticsCollectors, since others components may refer to them as expressions (as in "TVAG(ThisCSTAT)") and therefore the modeldatum must exist before checking such expression */
	// virtual void _createInternalAndAttachedData(); /*< A ModelDataDefinition or ModelComponent that includes (internal) ou refers to (attach) other ModelDataDefinition must register them inside this method. */
	/*! This method is not used yet. It should be usefull for new UIs */
	// virtual void _addSimulationControl(SimulationControl* property);


protected:
	void _doCreateReportStatisticsDataDefinitions();
	void _doCreateEditableDataDefinitions();
	void _doCreateOthersDataDefinitions();

private: /// new private user methods
	// ...

private: /// Attributes that should be loaded or saved with this component (Persistent Fields)

	/// Default values for the attributes. Used on initing, loading and saving
	const struct DEFAULT_VALUES {
		const std::string someString = "Test";
		const unsigned int someUint = 1;
	} DEFAULT;
	std::string _someString = DEFAULT.someString;
	unsigned int _someUint = DEFAULT.someUint;

private: /// internal DataElements (Composition)
	DummyElement* _internalDataDefinition = nullptr;

private: /// attached DataElements (Agrregation)
	// ...
};

