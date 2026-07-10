#ifndef METABOLICSUBMODELCOMPONENT_H
#define METABOLICSUBMODELCOMPONENT_H

#include <string>

#include "../../../kernel/simulator/model/ModelComponent.h"
#include "kernel/simulator/Plugin.h"
#include "plugins/data/WholeCellModeling/WholeCellState.h"

/**
 * Minimal metabolic submodel that produces ATP and NADPH each step.
 *
 * Replaces a full FBA step with a volume-proportional energy production
 * model calibrated for M. genitalium (Karr et al. 2012):
 *
 *   ATP_produced = atpYieldRate × cellVolume(fL) × deltaT  [mmol]
 *   ATP_pool(new) = min(ATP_pool + ATP_produced − ATP_consumed, atpPoolMax)
 *
 * ATP consumption is estimated as a fixed fraction of the current pool
 * (consumption rate = consumptionFraction × ATP_pool).
 *
 * Results are written to the WholeCellState metabolite pool under keys
 * "ATP" and "ADP". Downstream components can read "ATP" to gate reactions.
 *
 * For a full FBA integration, replace this component with a proper
 * MetabolicFluxBalance + network wiring. This component is intentionally
 * minimal so that Phases 2–5 can be validated without GLPK-specific setup.
 */
class MetabolicSubmodelComponent : public ModelComponent {
public:
	MetabolicSubmodelComponent(Model* model, std::string name = "");
	virtual ~MetabolicSubmodelComponent() override = default;

public:
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord* fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

public:
	void setWholeCellState(WholeCellState* state);
	WholeCellState* getWholeCellState() const;
	void setAtpYieldRate(double rate);
	double getAtpYieldRate() const;
	void setConsumptionFraction(double frac);
	double getConsumptionFraction() const;
	void setAtpPoolMax(double maxPool);
	double getAtpPoolMax() const;
	void setDeltaT(double dt);
	double getDeltaT() const;

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
	void _forwardEntity(Entity* entity);

private:
	const struct DEFAULT_VALUES {
		std::string wholeCellStateName = "";
		// mmol ATP per fL per second — calibrated for M. genitalium basal metabolism
		double atpYieldRate       = 1.0e-3;
		double consumptionFraction = 0.1;   // fraction of ATP pool consumed per step
		double atpPoolMax         = 10.0;   // mmol — approximate saturation
		double deltaT             = 60.0;   // s
	} DEFAULT;

	WholeCellState* _wholeCellState  = nullptr;
	double _atpYieldRate       = DEFAULT.atpYieldRate;
	double _consumptionFraction = DEFAULT.consumptionFraction;
	double _atpPoolMax         = DEFAULT.atpPoolMax;
	double _deltaT             = DEFAULT.deltaT;
};

#endif /* METABOLICSUBMODELCOMPONENT_H */
