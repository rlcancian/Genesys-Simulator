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

#include "Plugin.h"
#include "../util/List.h"
#include "Entity.h"
#include "ModelDataDefinition.h"
#include "ConnectionManager.h"
//namespace GenesysKernel {

class Model;
class Event;

/*!
 * A component of the model is a block that represents a specific behavior to be simulated. The behavior is triggered when an entity arrives at the component, which corresponds to the occurrence of an event. A simulation model corresponds to a set of interconnected components to form the process by which the entity is submitted.
 * @param model The model this component belongs to
 */
class ModelComponent : public ModelDataDefinition {
public: //! constructors
	ModelComponent(Model* model, std::string componentTypename, std::string name = "");
	virtual ~ModelComponent();

public: //! new public user methods for this component
	/*!
	 * \brief getConnections
	 * \return
	 */
    ConnectionManager* getConnectionManager() const; //!< Returns a list of components directly connected to the output. Usually the components have a single output, but they may have none (such as Dispose) or more than one (as Decide). In addition to the component, NextComponents specifies the inputPortNumber of the next component where the entity will be sent to. Ussually the components have a single input, but they may have none (such as Create) or more than one (as Match).
	/*!
	 * \brief hasBreakpointAt
	 * \return
	 */
	bool hasBreakpointAt();
	/*!
	 * \brief setDescription
	 * \param _description
	 */
	void setDescription(std::string _description);
	/*!
	 * \brief getDescription
	 * \return
	 */
	std::string getDescription() const;

    /*!
     * \brief remove
     */
    void remove(ModelComponent * comp);
	// ...

public: //! virtual public methods
	virtual std::string show();

public: //! static public methods that must have implementations (Load and New just the same. GetInformation must provide specific infos for the new component
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord *fields);
	// new static methods for all ModelComponents
	static bool Check(ModelComponent* component);
	static void CreateInternalData(ModelComponent* component);
	static void SaveInstance(PersistenceRecord *fields, ModelComponent* component);
	static void DispatchEvent(Event* event); //!< This method triggers the simulation of the behavior of the component. It is invoked when an event (corresponding to this component) is taken from the list of future events or when an entity arrives at this component by connection.

protected: //! virtual protected method that must be overriden
	virtual bool _loadInstance(PersistenceRecord *fields);
	virtual void _saveInstance(PersistenceRecord *fields, bool saveDefaultValues);
	// new virtual methods for all ModelComponents
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) = 0; //!< This method is only for ModelComponents, not ModelDataElements

protected: //! virtual protected methods that could be overriden by derived classes, if needed
	/*! This method is called by ModelChecker during model check. The component should check itself to verify if user parameters are ok (ex: correct syntax for the parser) and everithing in its parameters allow the model too run without errors in this component */
	// virtual bool _check(std::string* errorMessage);
	/*! This method returns all changes in the parser that are needed by plugins of this ModelDatas. When connecting a new plugin, ParserChangesInformation are used to change parser source code, whch is after compiled and dinamically linked to to simulator kernel to reflect the changes */
	// virtual ParserChangesInformation* _getParserChangesInformation();
	/*! This method is called by ModelSimulation when initianting the replication. The model should set all value for a new replication (Ex: setting back to 0 any internal counter, clearing lists, etc. */
	// virtual void _initBetweenReplications();
	/*! This method is called by ModelChecker and is necessary only for those components that instantiate internal elements that must exist before simulation starts and even before model checking. That's the case of components that have internal StatisticsCollectors, since others components may refer to them as expressions (as in "TVAG(ThisCSTAT)") and therefore the modeldatum must exist before checking such expression */
	// virtual void _createInternalAndAttachedData(); /*< A ModelDataDefinition or ModelComponent that includes (internal) ou refers to (attach) other ModelDataDefinition must register them inside this method. */
	/*! This method is not used yet. It should be usefull for new UIs */
	// virtual void _addProperty(PropertyBase* property);

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

