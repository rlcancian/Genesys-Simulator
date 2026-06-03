#ifndef CELLGROWTHCOMPONENT_H
#define CELLGROWTHCOMPONENT_H

#include <string>

#include "../../../kernel/simulator/model/ModelComponent.h"
#include "kernel/simulator/Plugin.h"
#include "plugins/data/WholeCellModeling/WholeCellState.h"

/**
 * Continuous cell growth applied at each simulation step.
 *
 * Implements exponential mass growth:
 *   mass(t+Δt) = mass(t) × (1 + growthRate × Δt)
 *   volume(fL) = (mass_g × 1e-3 / density_kg_m3) × 1e15
 *
 * Parameter defaults from CovertLab/WholeCell parameters.json (MIT):
 *   growthRate = 2.1393e-05 /s  (states.MetabolicReaction.meanInitialGrowthRate)
 *   density    = 1100.0 kg/m³   (states.Geometry.density)
 *   deltaT     = 60.0 s         (must match simulation clock step)
 */
class CellGrowthComponent : public ModelComponent {
public:
	CellGrowthComponent(Model* model, std::string name = "");
	virtual ~CellGrowthComponent() override = default;

public:
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord* fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

public:
	void setWholeCellState(WholeCellState* state);
	WholeCellState* getWholeCellState() const;
	void setGrowthRate(double rate);
	double getGrowthRate() const;
	void setDensity(double density);
	double getDensity() const;
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
		double growthRate = 2.1393e-05;  // /s — M. genitalium (parameters.json)
		double density    = 1100.0;      // kg/m³ — states.Geometry.density
		double deltaT     = 60.0;        // s — must match clock step
	} DEFAULT;

	WholeCellState* _wholeCellState = nullptr;
	double _growthRate = DEFAULT.growthRate;
	double _density    = DEFAULT.density;
	double _deltaT     = DEFAULT.deltaT;
};

#endif /* CELLGROWTHCOMPONENT_H */
