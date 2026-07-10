/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   SimulationModel.h
 * Author: Prof. Rafael Luiz Cancian, Dr. Eng.
 *
 * Created on 21 de Junho de 2018, 15:01
 */

#ifndef SIMULATIONMODEL_H
#define SIMULATIONMODEL_H

#include <list>
#include <string>

#include "../../util/List.h"
#include "ModelComponent.h"
#include "../Event.h"
#include "ModelChecker_if.h"
#include "../Parser_if.h"
#include "../persistence/Persistence_if.h"
#include "ModelDataManager.h"
#include "ModelComponentManager.h"
#include "../TraceManager.h"
#include "../OnEventManager.h"
#include "ModelInfo.h"
#include "ModelSimulation.h"
#include "../SimulationControlAndResponse.h"

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
	/*!
	 * \brief Creates a model bound to a simulator.
	 * \param simulator Parent simulator that owns the model.
	 * \param level Hierarchy level of the model within the simulator.
	 */
	Model(Simulator* simulator, unsigned int level = 0);
	/*!
	 * \brief Releases model-owned runtime state.
	 */
	virtual ~Model();
public: // model control
	//void showReports();
	/*!
	 * \brief Saves the model to a file.
	 * \param filename Output filename.
	 * \return \c true when saving succeeds.
	 */
	bool save(std::string filename);
	/*!
	 * \brief Loads the model from a file.
	 * \param filename Input filename.
	 * \return \c true when loading succeeds.
	 */
	bool load(std::string filename);
	/*!
	 * \brief Validates the model structure and configuration.
	 * \details Checks the integrity and consistency of the model, may correct
	 * some inconsistencies, and returns whether the model is ready to be
	 * simulated.
	 * \return \c true when the model is ready to simulate.
	 */
	bool check();
	/*!
	 * \brief Clears the current model content.
	 */
	void clear();
	/*!
	 * \brief Returns the model specification in the source language.
	 * \return Textual model specification.
	 */
	std::string showLanguage();
	/*!
	 * \brief Displays the model contents to the configured trace output.
	 */
	void show();
	/*!
	 * \brief Inserts a model data definition or component into the model.
	 * \details This is a generic entry point for either
	 * ComponentManager->insert() or ModelDataManager->insert().
	 * \param elemOrComp Object to insert.
	 * \return \c true when the object was accepted.
	 */
	bool insert(ModelDataDefinition* elemOrComp);
	/*!
	 * \brief Removes a model data definition or component from the model.
	 * \details This is a generic entry point for either
	 * ComponentManager->remove() or ModelDataManager->remove().
	 * \param elemOrComp Object to remove.
	 */
	void remove(ModelDataDefinition* elemOrComp);
	/*!
	 * \brief Computes the data definitions that should be removed together with a set of roots.
	 * \param roots Root objects selected for removal.
	 * \return Flattened list of dependent data definitions to remove as well.
	 */
	std::list<ModelDataDefinition*> collectDataDefinitionsRemovedWith(const std::list<ModelDataDefinition*>& roots) const;
	/*!
	 * \brief Creates an entity owned by this model.
	 * \param name Entity name.
	 * \param insertIntoModel If \c true, inserts the entity into the model immediately.
	 * \return Newly created entity.
	 */
	Entity* createEntity(std::string name, bool insertIntoModel = true);
	/*!
	 * \brief Removes an entity owned by this model.
	 * \param entity Entity to remove.
	 */
	void removeEntity(Entity* entity); //, bool collectStatistics);
	/*!
	 * \brief Schedules an entity transfer through a specific connection.
	 * \details Used by components to send entities to the next connected
	 * component, or by the model itself while processing an event.
	 * \param entity Entity to transfer.
	 * \param connection Destination connection.
	 * \param timeDelay Delay applied before the entity reaches the destination.
	 */
	void sendEntityToComponent(Entity* entity, Connection* connection, double timeDelay = 0.0);
	/*!
	 * \brief Schedules an entity transfer to a specific component and input port.
	 * \details Used by components to send entities to the next connected
	 * component, or by the model itself while processing an event.
	 * \param entity Entity to transfer.
	 * \param component Destination component.
	 * \param timeDelay Delay applied before the entity reaches the destination.
	 * \param componentinputPortNumber Destination input port number.
	 */
	void sendEntityToComponent(Entity* entity, ModelComponent* component, double timeDelay = 0.0, unsigned int componentinputPortNumber = 0);
	/*!
	 * \brief Parses and evaluates an expression.
	 * \details The parser always returns a double, even when the expression has
	 * syntax errors, in which case the result is 0.
	 * \param expression Expression text.
	 * \return Numeric result of the expression, or 0 on syntax failure.
	 */
	double parseExpression(const std::string expression);
	/*!
	 * \brief Parses and evaluates an expression while reporting success or failure.
	 * \details The parser always returns a double, even when the expression has
	 * syntax errors, in which case the result is 0. This overload also reports
	 * whether an error occurred.
	 * \param expression Expression text.
	 * \param success Output flag that receives the parsing result.
	 * \param errorMessage Output error message when parsing fails.
	 * \return Numeric result of the expression, or 0 on syntax failure.
	 */
    double parseExpression(const std::string expression, bool& success, std::string& errorMessage);
	/*!
	 * \brief Checks whether an expression is syntactically valid.
	 * \details This method is invoked by ModelComponents and ModelDatas in
	 * their private _check() methods to validate user-defined expressions.
	 * \param expression Expression text.
	 * \param expressionName Logical name used in diagnostics.
	 * \param errorMessage Output error message when validation fails.
	 * \return \c true when the expression is valid.
	 */
    bool checkExpression(const std::string expression, const std::string expressionName, std::string& errorMessage);
	/*!
	 * \brief Collects data-definition references found in an expression.
	 * \param expression Expression text.
	 * \param referencedDataDefinitions Output map of referenced data definitions by name.
	 */
	void checkReferencesToDataDefinitions(std::string expression, std::map<std::string, std::list<std::string>*>* referencedDataDefinitions);

public: // only gets
	/*!
	 * \brief Returns the unique model identifier.
	 * \return Model identifier.
	*/
	Util::identification getId() const;
	/*!
	 * \brief Indicates whether the model has pending changes.
	 * \return \c true when the model or one of its owned objects changed.
	 */
	bool hasChanged() const;
	/*!
	 * \brief Updates the changed flag for the model and its owned persistent objects.
	 * \param hasChanged New changed-state value.
	 */
	void setHasChanged(bool hasChanged);
	// 1:1
	/*!
	 * \brief Returns the event manager that coordinates model-level events.
	 * \details Provides access to the class that manages events generated by
	 * the model, such as the beginning of a new simulation or replication and
	 * the processing of an event.
	 * \return On-event manager instance.
	 */
    OnEventManager* getOnEventManager() const;
	/*!
	 * \brief Returns the data manager for model data definitions.
	 * \details Provides access to the class that manages the basic elements of
	 * the simulation model, such as queues, resources, variables and similar
	 * model data definitions.
	 * \return Data manager instance.
	 */
	ModelDataManager* getDataManager() const;
	/*!
	 * \brief Returns the component manager for model components.
	 * \return Component manager instance.
	 */
    ModelComponentManager* getComponentManager() const;
	/*!
	 * \brief Returns the model metadata object.
	 * \return Model info instance.
	 */
	ModelInfo* getInfos() const;
	/*!
	 * \brief Returns the parent simulator that owns this model.
	 * \return Parent simulator.
	 */
	Simulator* getParentSimulator() const;
	/*!
	 * \brief Returns the simulation controller associated with this model.
	 * \details Provides access to the class that manages the model simulation.
	 * \return Simulation controller instance.
	 */
	ModelSimulation* getSimulation() const;
	// 1:n
	/*!
	 * \brief Returns the future-event queue.
	 * \details The future events list is chronologically sorted. Events are
	 * scheduled by components while processing other events, and a replication
	 * evolves by sequentially processing the first event in this list. The list
	 * is initialized with the events first described by source components.
	 * \return Chronologically ordered list of scheduled events.
	 */
	List<Event*>* getFutureEvents() const;
	/*!
	 * \brief Returns the simulation controls exposed by the model.
	 * \details These are the values that can be externally controlled and
	 * usually correspond to input parameters that must be changed for an
	 * experimental design.
	 * \return List of writable simulation controls.
	 */
	List<SimulationControl*>* getControls() const;
	/*!
	 * \brief Returns the simulation responses exposed by the model.
	 * \details These are the exits or simulation results that can be read
	 * externally and usually correspond to statistics gathered for an
	 * experimental design.
	 * \return List of readable simulation responses.
	 */
	List<SimulationResponse*>* getResponses() const;

public: // gets and sets
	/*!
	 * \brief Sets the trace manager used by this model.
	 * \param _traceManager Trace manager instance.
	 */
	void setTracer(TraceManager* _traceManager);
	/*!
	 * \brief Returns the trace manager used by this model.
	 * \return Trace manager instance.
	 */
	TraceManager* getTracer() const;
	/*!
	 * \brief Returns the persistence interface used by this model.
	 * \return Persistence interface instance.
	 */
	Persistence_if* getPersistence() const;
	/*!
	 * \brief Returns the hierarchy level of the model.
	 * \details Provides access to the class that performs the trace of
	 * simulation and replications.
	 * \return Model level.
	 */
	unsigned int getLevel() const;

private:
	void _showConnections() const;
	void _showComponents() const;
	void _showElements() const;
	void _showSimulationControls() const;
	void _showSimulationResponses() const;
	void _destroyFutureEvents();
	void _destroyTransientEntities();
	void _destroyComponents();
	void _destroyModelDataDefinitions();
    void _clearOrphanedDataDefinitions();
    void _createInternalDataDefinitions();
private:
	bool _hasChanged = false;
private: // read only public access (gets)
	Util::identification _id;
	unsigned int _level = 0;
	Simulator* _parentSimulator; /*! The parent of the model is the simulator that contains this model*/
	// 1:1 (associted classes)
	TraceManager* _traceManager;
	OnEventManager* _eventManager;
	ModelDataManager* _modeldataManager;
	ModelComponentManager* _componentManager;
	ModelInfo* _modelInfo;
	ModelSimulation* _simulation;
	Persistence_if* _modelPersistence;

	// 1:n
	//List<ModelComponent*>* _components;
	List<Event*>* _futureEvents; //!< This is the calendar of future events, chronologically sorted, from where events are taken to be processed. This is one of the most important structures in Event driven simulation system
	// for process analyser
	List<SimulationResponse*>* _responses;
	List<SimulationControl*>* _controls;

private: // no public access (no gets / sets)
	ModelChecker_if* _modelChecker;
	Parser_if* _parser;
};
//namespace\\}
#endif /* SIMULATIONMODEL_H */
