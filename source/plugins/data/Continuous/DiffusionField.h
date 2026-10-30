#ifndef DIFFUSIONFIELD_H
#define DIFFUSIONFIELD_H

#include <string>
#include <vector>

#include "plugins/data/Continuous/ContinuousModelDataDefinition.h"
#include "kernel/simulator/PluginInformation.h"

class DiffusionMethodOfLinesSystem;

// Plugin that solves the diffusion (heat) equation du/dt = D*Laplacian(u) in N
// dimensions.
//
// It works in two steps. First the Method of Lines turns the PDE into a system
// of ODEs (central differences in space - see DiffusionMethodOfLinesSystem).
// Then it integrates that system in time with a solver from OdeSolverFactory, so
// the user can pick RK4 or Dormand-Prince 5(4), exactly like in BioNetwork.
//
// N is set by the Dimensions control; the grid has PointsPerDimension points and
// length DomainLength on each axis. The boundary is Dirichlet (fixed value) or
// Neumann (no flux, conserves mass), and the starting field is one of a few
// named shapes (sine mode, Gaussian bump, constant).
//
// The class follows the same layout as BioNetwork (controls, save/load, _check,
// simulate / advanceOneStep) to stay consistent with the rest of the code. The
// grid doesn't change during a run, so each step just rebuilds the small system
// object and reuses the stored field. The solver is never hard-coded - it comes
// from the OdeSolver name through the factory.
//
// Equation representation (design choice): the diffusion PDE is encoded as a
// dedicated C++ class (DiffusionMethodOfLinesSystem), not as a parser expression
// nor as dynamically compiled C++. This is a deliberate scope decision - a
// specific, well-tested class for diffusion - leaving a general user-defined
// equation mechanism (parser or CppCompiler) as a future extension.
//
// Continuous stepping and synchronization are controlled by the shared base.
class DiffusionField : public ContinuousModelDataDefinition {
public:
	struct StateView {
		const std::vector<double>& values;
		unsigned int dimensions;
		unsigned int pointsPerDimension;
		double totalMass;
		double maxValue;
		double l2Norm;
	};
	DiffusionField(Model* model, std::string name = "");
	virtual ~DiffusionField() override = default;

public:
	static ModelDataDefinition* LoadInstance(Model* model, PersistenceRecord* fields);
	static PluginInformation* GetPluginInformation();
	static ModelDataDefinition* NewInstance(Model* model, std::string name = "");

public:
	// --- configuration ---
	void setDimensions(unsigned int dimensions);
	unsigned int getDimensions() const;
	void setPointsPerDimension(unsigned int pointsPerDimension);
	unsigned int getPointsPerDimension() const;
	void setDomainLength(double domainLength);
	double getDomainLength() const;
	void setDiffusionCoefficient(double diffusionCoefficient);
	double getDiffusionCoefficient() const;
	void setBoundaryCondition(std::string boundaryCondition);
	std::string getBoundaryCondition() const;
	void setInitialCondition(std::string initialCondition);
	std::string getInitialCondition() const;
	void setInitialParameter(double initialParameter);
	double getInitialParameter() const;
	void setOdeSolver(std::string odeSolver);
	std::string getOdeSolver() const;
	// --- diagnostics (read-only outputs) ---
	double getTotalMass() const;
	double getMaxValue() const;
	double getL2Norm() const;
	const std::vector<double>& getField() const;
	/// Field value at a multi-index (size must equal Dimensions); 0 if invalid.
	double getFieldValue(const std::vector<unsigned int>& multiIndex) const;

	// Temporal execution is inherited from ContinuousModelDataDefinition.

	// --- display ---
	std::string showFieldAsAscii() const;

	virtual std::string show() override;

protected:
	virtual bool _loadInstance(PersistenceRecord* fields) override;
	virtual void _saveInstance(PersistenceRecord* fields, bool saveDefaultValues) override;
	virtual bool _check(std::string& errorMessage) override;
	void initializeContinuousState() override;
	bool evolveState(double currentTime, double dt, std::string& errorMessage) override;
	void onContinuousRunCompleted() override;
	std::any internalStateView() const override;
	void confirmContinuousState() override;

private:
	bool buildSystem(DiffusionMethodOfLinesSystem* system, std::string& errorMessage) const;
	void buildInitialField(const DiffusionMethodOfLinesSystem& system, std::vector<double>& field) const;
	void refreshDiagnostics(const DiffusionMethodOfLinesSystem& system);

private:
	const struct DEFAULT_VALUES {
		unsigned int dimensions = 2;
		unsigned int pointsPerDimension = 21;
		double domainLength = 1.0;
		double diffusionCoefficient = 0.1;
		std::string boundaryCondition = "Dirichlet";
		std::string initialCondition = "SineMode";
		double initialParameter = 1.0;
		std::string odeSolver = "RungeKutta4";
		std::string lastStatus = "Idle";
		std::string lastErrorMessage = "";
	} DEFAULT;

	unsigned int _dimensions = DEFAULT.dimensions;
	unsigned int _pointsPerDimension = DEFAULT.pointsPerDimension;
	double _domainLength = DEFAULT.domainLength;
	double _diffusionCoefficient = DEFAULT.diffusionCoefficient;
	std::string _boundaryCondition = DEFAULT.boundaryCondition;
	std::string _initialCondition = DEFAULT.initialCondition;
	double _initialParameter = DEFAULT.initialParameter;
	std::string _odeSolver = DEFAULT.odeSolver;

	std::vector<double> _field;       // current flattened grid state
	std::vector<double> _confirmedField;
	double _lastTotalMass = 0.0;
	double _lastMaxValue = 0.0;
	double _lastL2Norm = 0.0;
	double _confirmedTotalMass = 0.0;
	double _confirmedMaxValue = 0.0;
	double _confirmedL2Norm = 0.0;
};

#endif /* DIFFUSIONFIELD_H */
