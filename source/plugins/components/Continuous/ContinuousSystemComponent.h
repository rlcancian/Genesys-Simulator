/*
 * File:   ContinuousSystemComponent.h
 * Author: GenESyS
 *
 * ModelComponent that drives continuous-time simulation by delegating
 * numerical integration to an ODESolver data definition.
 */

#ifndef CONTINUOUSSYSTEMCOMPONENT_H
#define CONTINUOUSSYSTEMCOMPONENT_H

#include <string>

#include "kernel/simulator/model/ModelComponent.h"
#include "plugins/data/Continuous/ODESolver.h"

/*!
 * \brief Component that advances a continuous ODE system when entities arrive.
 *
 * ContinuousSystemComponent holds a reference to an ODESolver data definition
 * and triggers integration each time an entity enters through port 0.  The
 * solved state is written back to the ODESolver so downstream components can
 * read the updated values.  The entity is forwarded through output port 0
 * after integration completes.
 */
class ContinuousSystemComponent : public ModelComponent {
public: // constructors
	ContinuousSystemComponent(Model* model, std::string name = "");
	virtual ~ContinuousSystemComponent() override = default;

public: // virtual
	virtual std::string show() override;

public: // static — required by the plugin framework
	static PluginInformation*  GetPluginInformation();
	static ModelComponent*     LoadInstance(Model* model, PersistenceRecord* fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");
	static void                SaveInstance(PersistenceRecord* fields, ModelComponent* component);
	static bool                Check(ModelComponent* component);
	static void                CreateInternalData(ModelComponent* component);
	static void                DispatchEvent(Event* event);

public: // getters & setters
	void        setOdeSolver(ODESolver* solver);
	ODESolver*  getOdeSolver() const;
	void        setOdeSolverName(std::string name);
	std::string getOdeSolverName() const;

protected: // virtual — must be overridden
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override;
	virtual bool _loadInstance(PersistenceRecord* fields) override;
	virtual void _saveInstance(PersistenceRecord* fields, bool saveDefaultValues) override;
	virtual bool _check(std::string& errorMessage) override;
	virtual void _createEditableDataDefinitions() override;

private:
	const struct DEFAULT_VALUES {
		const std::string odeSolverName = "";
	} DEFAULT;

	ODESolver*  _odeSolver     = nullptr;
	std::string _odeSolverName = DEFAULT.odeSolverName;
};

#endif /* CONTINUOUSSYSTEMCOMPONENT_H */
