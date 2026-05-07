/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   Element.h
 * Author: rafael.luiz.cancian
 *
 * Created on 21 de Junho de 2018, 15:56
 */

#ifndef MODELCOMPONENT_H
#define MODELCOMPONENT_H

#include <string>
#include <list>

#include "../Plugin.h"
#include "../../util/List.h"
#include "../Entity.h"
#include "ModelDataDefinition.h"
#include "../ConnectionManager.h"
//namespace GenesysKernel {

class Model;
class Event;

/*!
 * A component of the model is a block that represents a specific behavior to be simulated. The behavior is triggered when an entity arrives at the component, which corresponds to the occurrence of an event. A simulation model corresponds to a set of interconnected components to form the process by which the entity is submitted.
 * @param model The model this component belongs to
 */
class ModelComponent : public ModelDataDefinition {
public: //! constructors
	/*!
	 * \brief Creates a model component and registers it in the component manager.
	 * \param model Parent model.
	 * \param componentTypename Runtime type name for the concrete component.
	 * \param name Initial component name.
	 */
	ModelComponent(Model* model, std::string componentTypename, std::string name = "");
	/*!
	 * \brief Releases component-owned resources and unregisters it from the model.
	 */
	virtual ~ModelComponent();

public: //! new public user methods for this component
	/*!
	 * \brief Returns the connection manager owned by this component.
	 * \details This manager stores the components directly connected to the
	 * output. Usually a component has a single output, but it may have none
	 * (such as Dispose) or more than one (such as Decide). Along with each
	 * connected component, the manager stores the input port number where the
	 * entity will be sent. Usually components have a single input, but they may
	 * have none (such as Create) or more than one (such as Match).
	 * \return Connection manager that stores outgoing connections.
	 */
    ConnectionManager* getConnectionManager() const;
    /*!
	 * \brief Connects this component to another component.
	 * \param component Target component.
	 * \param inputPortNumber Input port number of the target component.
	 */
    void connectTo(ModelComponent* component, unsigned int inputPortNumber = 0);
    /*!
	 * \brief Indicates whether a simulation breakpoint is set on this component.
	 * \return \c true when the simulation breakpoint list contains this component.
	 */
	bool hasBreakpointAt();
	/*!
	 * \brief Updates the free-form description shown by the GUI.
	 * \param _description New description text.
	 */
	void setDescription(std::string _description);
	/*!
	 * \brief Returns the free-form description associated with this component.
	 * \return Description text.
	 */
	std::string getDescription() const;

    /*!
     * \brief Removes a connected component from this component.
     * \param comp Component to remove from the outgoing connection set.
     */
    void remove(ModelComponent * comp);
	// ...

public: //! virtual public methods
	/*!
	 * \brief Returns a textual representation of this component.
	 * \return Human-readable description used for tracing and debugging.
	 */
	virtual std::string show() override;

public: //! static public methods that must have implementations (Load and New just the same. GetInformation must provide specific infos for the new component
	/*!
	 * \brief Loads a component instance from serialized fields.
	 * \param model Parent model that will own the component.
	 * \param fields Serialized fields for the component.
	 * \return Newly created component instance.
	 */
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	// new static methods for all ModelComponents
	/*!
	 * \brief Validates a component instance.
	 * \param component Component to validate.
	 * \return \c true when the component is valid.
	 */
	static bool Check(ModelComponent* component);
	/*!
	 * \brief Creates internal data required by a component instance.
	 * \param component Component whose related data should be created.
	 */
	static void CreateInternalData(ModelComponent* component);
	/*!
	 * \brief Serializes a component instance into a persistence record.
	 * \param fields Output persistence record.
	 * \param component Component to serialize.
	 */
	static void SaveInstance(PersistenceRecord *fields, ModelComponent* component);
	/*!
	 * \brief Dispatches a simulation event to the target component.
	 * \details This method triggers the simulation behavior of the component.
	 * It is invoked when an event corresponding to this component is taken from
	 * the future events list or when an entity arrives through a connection.
	 * \param event Event to process.
	 */
	static void DispatchEvent(Event* event);

protected: //! virtual protected method that must be overriden
	virtual bool _loadInstance(PersistenceRecord *fields) override;
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues) override;
	// new virtual methods for all ModelComponents
	/*!
	 * \brief Handles a dispatched event for this component type.
	 * \details This method is only for ModelComponents, not for
	 * ModelDataElements.
	 * \param entity Entity being processed.
	 * \param inputPortNumber Input port where the entity arrived.
	 */
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) = 0;

protected: //! virtual protected methods that could be overriden by derived classes, if needed
	/*! This method is called by ModelChecker during model check. The component should check itself to verify if user parameters are ok (ex: correct syntax for the parser) and everithing in its parameters allow the model too run without errors in this component */
	// virtual bool _check(std::string& errorMessage);
	/*! This method returns all changes in the parser that are needed by plugins of this ModelDatas. When connecting a new plugin, ParserChangesInformation are used to change parser source code, whch is after compiled and dinamically linked to to simulator kernel to reflect the changes */
	// virtual ParserChangesInformation* _getParserChangesInformation();
	/*! This method is called by ModelSimulation when initianting the replication. The model should set all value for a new replication (Ex: setting back to 0 any internal counter, clearing lists, etc. */
	// virtual void _initBetweenReplications();
	/*! Legacy compatibility hook. Prefer the granular create* methods and the composed association manager. */
	// virtual void _createInternalAndAttachedData(); /* legacy combined lifecycle hook */
	/*! This method is not used yet. It should be usefull for new UIs */
	// virtual void _addSimulationControl(SimulationControl* property);

protected: // new protected attributes for all ModelComponents
	ConnectionManager* _connections = new ConnectionManager();

private: //! new private user methods
	// ...

private: //! Attributes that should be loaded or saved with this component (Persistent Fields)

	// Default values for the attributes. Used on initing, loading and saving
	const struct DEFAULT_VALUES {
		const unsigned int nextSize = 1;              // No need for attribute. Taken from _connections
		const unsigned int nextinputPortNumber = 0;
		const std::string description = "";
	} DEFAULT;
	std::string _description = DEFAULT.description;

private: //! internal DataElements (Composition)
	//...

private: //! attached DataElements (Agrregation)
	// ...

};
//namespace\\}
#endif /* MODELCOMPONENT_H */
