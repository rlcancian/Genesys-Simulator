/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:    ModalModelDefault.h
 * Author: Prof. Rafael Luiz Cancian, Dr. Eng.
 *
 * Created on 01 de Julho de 2025, 14:26
 */

#pragma once

#include "../../kernel/simulator/ModelComponent.h"
#include "network/DefaultNode.h"
/*!
 This component ...
 */
class ModalModelDefault : public ModelComponent {
public: /// constructors
        ModalModelDefault(Model* model, std::string name = "");
    virtual ~    ModalModelDefault() = default;

public: // getters and setters
	virtual void setMaxTransitionsPerDispatch(unsigned int maxTransitionsPerDispatch);
	virtual unsigned int getMaxTransitionsPerDispatch() const;
	virtual List<DefaultNode*>* getNodes() const;
	virtual List<DefaultNodeTransition*>* getTransitions() const;
	DefaultNode* getEntryNode();
	void setEntryNode(DefaultNode* const entry_node);
	DefaultNode* getCurrentNode();
	std::string getTimeDelayExpressionPerDispatch();
	void setTimeDelayExpressionPerDispatch(const std::string time_delay_expression_per_dispatch);
	Util::TimeUnit getTimeDelayPerDispatchTimeUnit();
	void setTimeDelayPerDispatchTimeUnit(const Util::TimeUnit time_delay_per_dispatch_time_unit);

public: /// new public user methods for this component
	virtual void addNode(DefaultNode* node);
	virtual void removeNode(DefaultNode* node);
	virtual void addTransition(DefaultNodeTransition* transition);
	virtual void removeTransition(DefaultNodeTransition* transition);
	virtual void addOutputExpressionReference(ModelDataDefinition* expressionReference);
	virtual void removeOutputExpressionReference(DefaultNodeTransition* expressionReference);

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
	virtual bool _check(std::string& errorMessage) override;
	/*! This method returns all changes in the parser that are needed by plugins of this ModelDatas. When connecting a new plugin, ParserChangesInformation are used to change parser source code, whch is after compiled and dinamically linked to to simulator kernel to reflect the changes */
	// virtual ParserChangesInformation* _getParserChangesInformation();
	/*! This method is called by ModelSimulation when initianting the replication. The model should set all value for a new replication (Ex: setting back to 0 any internal counter, clearing lists, etc. */
	virtual void _initBetweenReplications() override;
	/*! This method is called by ModelChecker and is necessary only for those components that instantiate internal elements that must exist before simulation starts and even before model checking. That's the case of components that have internal StatisticsCollectors, since others components may refer to them as expressions (as in "TVAG(ThisCSTAT)") and therefore the modeldatum must exist before checking such expression */
	virtual void _createInternalAndAttachedData() override; /*< A ModelDataDefinition or ModelComponent that includes (internal) ou refers to (attach) other ModelDataDefinition must register them inside this method. */
	/*! This method is not used yet. It should be usefull for new UIs */
	// virtual void _addProperty(SimulationControl* property);

// sections bellow are *protected* since ModalModelDefault is intended to be extended
protected: /// Attributes that should be loaded or saved with this component (Persistent Fields)

	/// Default values for the attributes. Used on initing, loading and saving
	const struct DEFAULT_VALUES {
		const std::string timeDelayExpressionPerDispatch = "0";
		const Util::TimeUnit timeDelayPerDispatchTimeUnit = Util::TimeUnit::second;
		const unsigned int maxTransitionsPerDispatch = 1;
	} DEFAULT;
	//std::string _entryNodeName = DEFAULT.entryNodeName;
	unsigned int _maxTransitionsPerDispatch = DEFAULT.maxTransitionsPerDispatch;
	std::string _timeDelayExpressionPerDispatch = DEFAULT.timeDelayExpressionPerDispatch;
	Util::TimeUnit _timeDelayPerDispatchTimeUnit = DEFAULT.timeDelayPerDispatchTimeUnit;

protected:
	DefaultNode* _currentNode = nullptr;
    DefaultNode* _entryNode = nullptr;

protected: /// internal COMPONENTS (since it's a modal model / network) (Composition)
	List<DefaultNode*>* _nodes = new List<DefaultNode*>();
	List<DefaultNodeTransition*>* _transitions = new List<DefaultNodeTransition*>();
	//List<ModelDataDefinition*>* _expressionDataReferences = new List<ModelDataDefinition*>();


private: /// internal DataElements (Composition)
	// ...
private: /// attached DataElements (Agrregation)
	// ...

private: /// new private user methods
	// ...


};
