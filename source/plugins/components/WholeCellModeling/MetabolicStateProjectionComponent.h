#ifndef METABOLICSTATEPROJECTIONCOMPONENT_H
#define METABOLICSTATEPROJECTIONCOMPONENT_H

#include <string>
#include <vector>

#include "../../../kernel/simulator/model/ModelComponent.h"
#include "kernel/simulator/Plugin.h"
#include "plugins/components/BiochemicalSimulation/MetabolicFluxBalance.h"
#include "plugins/data/WholeCellModeling/WholeCellState.h"

/*!
 * \brief Projects flux-balance outputs into a WholeCellState.
 *
 * MetabolicStateProjectionComponent bridges FBA-style submodels with the
 * whole-cell shared state. It reads the latest objective value and reaction
 * fluxes computed by a MetabolicFluxBalance component and writes selected
 * projections into WholeCellState as:
 * - global metabolite proxies;
 * - compartment-specific metabolite proxies;
 * - pathway activity summaries.
 *
 * This component intentionally does not solve the metabolic optimization
 * itself. It only transfers selected results to the whole-cell layer so that
 * growth, checkpoints, and gene-expression submodels can consume them.
 */
class MetabolicStateProjectionComponent : public ModelComponent {
public:
	enum class ProjectionKind {
		MetaboliteAmount,
		CompartmentMetaboliteAmount,
		PathwayActivity
	};

	enum class ProjectionUpdateMode {
		Overwrite,
		Accumulate,
		Turnover
	};

	struct Projection {
		std::string reactionName;
		std::string targetKey;
		ProjectionKind kind = ProjectionKind::MetaboliteAmount;
		ProjectionUpdateMode updateMode = ProjectionUpdateMode::Overwrite;
		double scale = 1.0;
		double offset = 0.0;
		double turnoverFraction = 0.0;
		std::string compartmentName;
	};

public:
	MetabolicStateProjectionComponent(Model* model, std::string name = "");
	virtual ~MetabolicStateProjectionComponent() override = default;

public:
	static PluginInformation* GetPluginInformation();
	static ModelComponent* LoadInstance(Model* model, PersistenceRecord* fields);
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

public:
	void setWholeCellState(WholeCellState* state);
	WholeCellState* getWholeCellState() const;
	void setFluxBalanceComponent(MetabolicFluxBalance* component);
	MetabolicFluxBalance* getFluxBalanceComponent() const;
	void setFluxBalanceComponentName(std::string componentName);
	std::string getFluxBalanceComponentName() const;

	void clearObjectiveProjection();
	void setObjectiveAsMetabolite(const std::string& targetKey, double scale = 1.0, double offset = 0.0,
	                              ProjectionUpdateMode updateMode = ProjectionUpdateMode::Overwrite,
	                              double turnoverFraction = 0.0);
	void setObjectiveAsCompartmentMetabolite(const std::string& compartmentName, const std::string& targetKey,
	                                         double scale = 1.0, double offset = 0.0,
	                                         ProjectionUpdateMode updateMode = ProjectionUpdateMode::Overwrite,
	                                         double turnoverFraction = 0.0);
	void setObjectiveAsPathwayActivity(const std::string& targetKey, double scale = 1.0, double offset = 0.0,
	                                   ProjectionUpdateMode updateMode = ProjectionUpdateMode::Overwrite,
	                                   double turnoverFraction = 0.0);
	bool hasObjectiveProjection() const;

	void clearFluxProjections();
	void addMetaboliteProjection(const std::string& reactionName, const std::string& targetKey,
	                             double scale = 1.0, double offset = 0.0,
	                             ProjectionUpdateMode updateMode = ProjectionUpdateMode::Overwrite,
	                             double turnoverFraction = 0.0);
	void addCompartmentMetaboliteProjection(const std::string& reactionName, const std::string& compartmentName,
	                                        const std::string& targetKey, double scale = 1.0, double offset = 0.0,
	                                        ProjectionUpdateMode updateMode = ProjectionUpdateMode::Overwrite,
	                                        double turnoverFraction = 0.0);
	void addPathwayProjection(const std::string& reactionName, const std::string& targetKey,
	                          double scale = 1.0, double offset = 0.0,
	                          ProjectionUpdateMode updateMode = ProjectionUpdateMode::Overwrite,
	                          double turnoverFraction = 0.0);
	unsigned int getProjectionCount() const;

	void setLastSucceeded(bool lastSucceeded);
	bool getLastSucceeded() const;
	void setLastMessage(std::string lastMessage);
	std::string getLastMessage() const;

	virtual std::string show() override;

protected:
	virtual bool _loadInstance(PersistenceRecord* fields) override;
	virtual void _saveInstance(PersistenceRecord* fields, bool saveDefaultValues) override;
	virtual bool _check(std::string& errorMessage) override;
	virtual void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override;
	virtual void _createEditableDataDefinitions() override;

private:
	void _forwardEntity(Entity* entity);
	void _syncEditableDataDefinitions();
	void _resolveFluxBalanceComponent();
	void _configureObjectiveProjection(ProjectionKind kind, const std::string& targetKey, const std::string& compartmentName,
	                                   double scale, double offset, ProjectionUpdateMode updateMode,
	                                   double turnoverFraction);
	void _appendProjection(const Projection& projection);
	void _applyProjection(const Projection& projection, double value);
	std::string _resolveCompartmentName(const Projection& projection) const;
	double _resolveCurrentValue(const Projection& projection) const;

private:
	const struct DEFAULT_VALUES {
		std::string wholeCellStateName = "";
		std::string fluxBalanceComponentName = "";
		bool objectiveProjectionEnabled = false;
		std::string objectiveTargetKey = "";
		std::string objectiveKind = "MetaboliteAmount";
		std::string objectiveUpdateMode = "Overwrite";
		double objectiveScale = 1.0;
		double objectiveOffset = 0.0;
		double objectiveTurnoverFraction = 0.0;
		std::string objectiveCompartmentName = "";
		bool lastSucceeded = false;
		std::string lastMessage = "";
	} DEFAULT;

	WholeCellState* _wholeCellState = nullptr;
	MetabolicFluxBalance* _fluxBalanceComponent = nullptr;
	std::string _fluxBalanceComponentName = DEFAULT.fluxBalanceComponentName;
	bool _objectiveProjectionEnabled = DEFAULT.objectiveProjectionEnabled;
	Projection _objectiveProjection;
	std::vector<Projection> _projections;
	bool _lastSucceeded = DEFAULT.lastSucceeded;
	std::string _lastMessage = DEFAULT.lastMessage;
};

#endif /* METABOLICSTATEPROJECTIONCOMPONENT_H */
