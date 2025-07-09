/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   SimulationModel.h
 * Author: rafael.luiz.cancian
 *
 * Created on 21 de Junho de 2018, 15:01
 */

#ifndef SIMULATIONMODEL_H
#define SIMULATIONMODEL_H

#include <string>

#include "../util/List.h"
#include "ModelComponent.h"
#include "Event.h"
#include "ModelChecker_if.h"
#include "Parser_if.h"
#include "ModelPersistence_if.h"
#include "ModelDataManager.h"
#include "ComponentManager.h"
#include "TraceManager.h"
#include "OnEventManager.h"
#include "ModelInfo.h"
#include "ModelSimulation.h"
//for PAN
#include "PropertyGenesys.h"

//namespace GenesysKernel {
class Simulator;

/*!
 * Model is probably the most important class of Genesys kernel.
 * It represents a discrete event-driven simulation model.
 * Each model is responsible for controlling its own simulation, ie, for sequentially processing events and collecting statistical results.
 * A model is mainly represented by a collection of components (ModelComponent), adequately configurated and connected, and a collection of under layered modeldatum (ModelDataDefinition).
 */
class Model {
public:
	Model(Simulator* simulator, unsigned int level = 0);
	virtual ~Model() = default;
public: // model control
	//void showReports();
	/*!
	 * \brief save
	 * \param filename
	 * \return
	 */
	bool save(std::string filename);
	/*!
	 * \brief load
	 * \param filename
	 * \return
	 */
	bool load(std::string filename);
	/*!
	 * \brief check
	 * \return
	 */
	bool check(); //!< Checks the integrity and consistency of the model, possibly corrects some inconsistencies, and returns if the model is in position to the simulated.
	/*!
	 * \brief clear
	 */
	void clear();
	/*!
	 * \brief showLanguage
	 * \return
	 */
	std::string showLanguage();
	/*!
	 * \brief show
	 */
	void show();
	/*!
	 * \brief insert
	 * \param elemOrComp
	 * \return
	 */
	bool insert(ModelDataDefinition* elemOrComp); //!< Insert a new ModelDataDefinition or ModelComponent into the model (since 20191015). It's a generic access to ComponentManager->insert() or ModelDatao->insert()
	/*!
	 * \brief remove
	 * \param elemOrComp
	 */
	void remove(ModelDataDefinition* elemOrComp); //!< Remove a new ModelDataDefinition or ModelComponent into the model (since 20191015). It's a generic access to ComponentManager->remove() or ModelDatao->remove()
	/*!
	 * \brief createEntity
	 * \param name
	 * \param insertIntoModel
	 * \return
	 */
	Entity* createEntity(std::string name, bool insertIntoModel = true);
	/*!
	 * \brief removeEntity
	 * \param entity
	 */
	void removeEntity(Entity* entity); //, bool collectStatistics);
	/*!
	 * \brief sendEntityToComponent
	 * \param entity
	 * \param connection
	 * \param timeDelay
	 */
	void sendEntityToComponent(Entity* entity, Connection* connection, double timeDelay = 0.0); //!< Used by components (ModelComponent) to send entities to another specific component, usually the next one connected to it, or used by the model itself, when processing an event (Event).
	/*!
	 * \brief sendEntityToComponent
	 * \param entity
	 * \param component
	 * \param timeDelay
	 * \param componentinputPortNumber
	 */
	void sendEntityToComponent(Entity* entity, ModelComponent* component, double timeDelay = 0.0, unsigned int componentinputPortNumber = 0); //!< Used by components (ModelComponent) to send entities to another specific component, usually the next one connected to it, or used by the model itself, when processing an event (Event).
	/*!
	 * \brief parseExpression
	 * \param expression
	 * \return
	 */
	double parseExpression(const std::string expression); //!< Invokes the parser to evaluate tyhe expression. Result is always a double, even if expression has syntatic errors (returns 0)
	/*!
	 * \brief parseExpression
	 * \param expression
	 * \param success
	 * \param errorMessage
	 * \return
	 */
	double parseExpression(const std::string expression, bool* success, std::string* errorMessage); //!< Invokes the parser to evaluate tyhe expression. Result is always a double, even if expression has syntatic errors (returns 0). Explicitly informs if there was an error
	/*!
	 * \brief checkExpression
	 * \param expression
	 * \param expressionName
	 * \param errorMessage
	 * \return
	 */
	bool checkExpression(const std::string expression, const std::string expressionName, std::string* errorMessage); //!< This is invoked by ModelComponents and ModelDatas in their private method _check() to verify if an expression defined by user is valid or not
	/*!
	 * \brief checkReferencesToDataDefinitions
	 * \param expression
	 * \param referencedDataDefinitions
	 */
	void checkReferencesToDataDefinitions(std::string expression, std::map<std::string, std::list<std::string>*>* referencedDataDefinitions);
public: // only gets
	/*!
	 * \brief getId
	 * \return
	*/
	Util::identification getId() const;
	/*!
	 * \brief hasChanged
	 * \return
	 */
	bool hasChanged() const;
	// 1:1
	/*!
	 * \brief getOnEvents
	 * \return
	 */
    OnEventManager* getOnEventManager() const; //!< Provides acccess to the class that manages events generated by the model, such as the beggining of a new simulation or replication, the processig of an event and much more
	/*!
	 * \brief getDataManager
	 * \return
	 */
	ModelDataManager* getDataManager() const; //!< Provides access to the class that manages the most basic elements of the simulation model (such as queues, resources, variables, etc.).
	/*!
	 * \brief getComponents
	 * \return
	 */
    ComponentManager* getComponentManager() const;
	/*!
	 * \brief getInfos
	 * \return
	 */
	ModelInfo* getInfos() const;
	/*!
	 * \brief getParentSimulator
	 * \return
	 */
	Simulator* getParentSimulator() const;
	/*!
	 * \brief getSimulation
	 * \return
	 */
	ModelSimulation* getSimulation() const; //!< Provides access to the class that manages the model simulation.
	// 1:n
	/*!
	 * \brief getFutureEvents
	 * \return
	 */
	List<Event*>* getFutureEvents() const; //!< The future events list chronologically sorted; Events are scheduled by components when processing other events, and a replication evolves over time by sequentially processing the very first event in this list. It's initialized with events first described by source components (SourceComponentModel).
	/*!
	 * \brief getControls
	 * \return
	 */
	List<SimulationControl*>* getControls() const; //!< Returns a list of values that can be externally controlled (changed). They usually correspond to input parameters in the simulation model that must be changed for an experimental design.
	/*!
	 * \brief getResponses
	 * \return
	 */
	List<SimulationControl*>* getResponses() const; //!< Returns a list of exits or simulation results that can be read externally. They usually correspond to statistics resulting from the simulation that must be read for an experiment design.

public: // gets and sets
	/*!
	 * \brief setTracer
	 * \param _traceManager
	 */
	void setTracer(TraceManager* _traceManager);
	/*!
	 * \brief getTracer
	 * \return
	 */
	TraceManager* getTracer() const;
	/*!
	 * \brief getPersistence
	 * \return
	 */
	ModelPersistence_if* getPersistence() const;
	/*!
	 * \brief setAutomaticallyCreatesModelDataDefinitions
	 * \param _automaticallyCreatesModelDataDefinitions
	 */
	void setAutomaticallyCreatesModelDataDefinitions(bool _automaticallyCreatesModelDataDefinitions);
	/*!
	 * \brief isAutomaticallyCreatesModelDataDefinitions
	 * \return
	 */
	bool isAutomaticallyCreatesModelDataDefinitions() const;
	/*!
	 * \brief getLevel
	 * \return
	 */
	unsigned int getLevel() const; //!< Provides access to the class that performs the trace of simulation and replications.

private:
	void _showConnections() const;
	void _showComponents() const;
	void _showElements() const;
	void _showSimulationControls() const;
	void _showSimulationResponses() const;
	void _createModelInternalElements();
private:
	bool _hasChanged = false;
	bool _isChecked = false; // @TODO: Not implemented yet. First, _hasChanged should be trustful
	bool _automaticallyCreatesModelDataDefinitions; // default will come from Traits in the constructor
private: // read only public access (gets)
	Util::identification _id;
	unsigned int _level = 0;
	Simulator* _parentSimulator; /*! The parent of the model is the simulator that contains this model*/
	// 1:1 (associted classes)
	TraceManager* _traceManager;
	OnEventManager* _eventManager;
	ModelDataManager* _modeldataManager;
	ComponentManager* _componentManager;
	ModelInfo* _modelInfo;
	ModelSimulation* _simulation;
	ModelPersistence_if* _modelPersistence;

	// 1:n
	//List<ModelComponent*>* _components;
	List<Event*>* _futureEvents; //!< This is the calendar of future events, chronologically sorted, from where events are taken to be processed. This is one of the most important structures in Event driven simulation system
	// for process analyser
	List<SimulationControl*>* _responses;
	List<SimulationControl*>* _controls;

private: // no public access (no gets / sets)
	ModelChecker_if* _modelChecker;
	Parser_if* _parser;
};
//namespace\\}
#endif /* SIMULATIONMODEL_H */

