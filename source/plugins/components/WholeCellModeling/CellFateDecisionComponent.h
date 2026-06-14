#ifndef CELLFATEDECISIONCOMPONENT_H
#define CELLFATEDECISIONCOMPONENT_H

#include <string>

#include "../../../kernel/simulator/model/ModelComponent.h"
#include "kernel/simulator/Plugin.h"
#include "plugins/data/WholeCellModeling/WholeCellState.h"

/*!
 * \brief Routes entities according to the current whole-cell lifecycle state.
 *
 * CellFateDecisionComponent converts the coarse lifecycle annotation stored in
 * WholeCellState into explicit simulation branching:
 * - port 0: viable default flow
 * - port 1: division-ready flow
 * - port 2: starved-but-still-viable flow
 * - port 3: dead or non-viable flow
 *
 * The component does not mutate biological state. It only reads viability and
 * lifecycle phase from WholeCellState and routes the entity to the appropriate
 * downstream component.
 */
class CellFateDecisionComponent : public ModelComponent {
public:
	CellFateDecisionComponent(Model* model, std::string name = "");
	virtual ~CellFateDecisionComponent() override = default;

public:
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord* fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

public:
	void setWholeCellState(WholeCellState* state);
	WholeCellState* getWholeCellState() const;
	void setDivisionReadyPhase(std::string phase);
	std::string getDivisionReadyPhase() const;
	void setStarvedPhase(std::string phase);
	std::string getStarvedPhase() const;
	void setArrestedPhase(std::string phase);
	std::string getArrestedPhase() const;
	void setDeadPhase(std::string phase);
	std::string getDeadPhase() const;
	unsigned int getLastRoutedPort() const;

	virtual std::string show() override;

protected:
	virtual bool _loadInstance(PersistenceRecord* fields) override;
	virtual void _saveInstance(PersistenceRecord* fields, bool saveDefaultValues) override;
	virtual bool _check(std::string& errorMessage) override;
	virtual void _initBetweenReplications() override;
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override;

protected:
	virtual void _createEditableDataDefinitions() override;

private:
	unsigned int _selectOutputPort() const;
	void _forwardEntity(Entity* entity, unsigned int port);

private:
	const struct DEFAULT_VALUES {
		std::string wholeCellStateName = "";
		std::string divisionReadyPhase = "division_ready";
		std::string starvedPhase = "starved";
		std::string arrestedPhase = "arrested";
		std::string deadPhase = "dead";
		unsigned int lastRoutedPort = 0u;
	} DEFAULT;

	WholeCellState* _wholeCellState = nullptr;
	std::string _divisionReadyPhase = DEFAULT.divisionReadyPhase;
	std::string _starvedPhase = DEFAULT.starvedPhase;
	std::string _arrestedPhase = DEFAULT.arrestedPhase;
	std::string _deadPhase = DEFAULT.deadPhase;
	unsigned int _lastRoutedPort = DEFAULT.lastRoutedPort;
};

#endif /* CELLFATEDECISIONCOMPONENT_H */
