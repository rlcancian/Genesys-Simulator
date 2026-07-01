#ifndef DIFFUSIONFIELD_H
#define DIFFUSIONFIELD_H

#include <string>
#include <vector>

#include "../../../kernel/simulator/model/ModelDataDefinition.h"
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
class DiffusionField : public ModelDataDefinition {
public:
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
	void setStartTime(double startTime);
	double getStartTime() const;
	void setStopTime(double stopTime);
	double getStopTime() const;
	void setStepSize(double stepSize);
	double getStepSize() const;
	void setCurrentTime(double currentTime);
	double getCurrentTime() const;
	void setOdeSolver(std::string odeSolver);
	std::string getOdeSolver() const;
	void setLastStatus(std::string lastStatus);
	std::string getLastStatus() const;
	void setLastErrorMessage(std::string lastErrorMessage);
	std::string getLastErrorMessage() const;

	// --- diagnostics (read-only outputs) ---
	double getTotalMass() const;
	double getMaxValue() const;
	double getL2Norm() const;
	const std::vector<double>& getField() const;
	/// Field value at a multi-index (size must equal Dimensions); 0 if invalid.
	double getFieldValue(const std::vector<unsigned int>& multiIndex) const;

	// --- execution ---
	bool simulate(std::string& errorMessage);
	bool simulate(double startTime, double stopTime, double stepSize, std::string& errorMessage);
	bool advanceOneStep(std::string& errorMessage);

	// --- display ---
	std::string showFieldAsAscii() const;

	virtual std::string show() override;

protected:
	virtual bool _loadInstance(PersistenceRecord* fields) override;
	virtual void _saveInstance(PersistenceRecord* fields, bool saveDefaultValues) override;
	virtual bool _check(std::string& errorMessage) override;
	virtual void _initBetweenReplications() override;

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
		double startTime = 0.0;
		double stopTime = 0.1;
		double stepSize = 0.01;
		double currentTime = 0.0;
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
	double _startTime = DEFAULT.startTime;
	double _stopTime = DEFAULT.stopTime;
	double _stepSize = DEFAULT.stepSize;
	double _currentTime = DEFAULT.currentTime;
	std::string _odeSolver = DEFAULT.odeSolver;
	std::string _lastStatus = DEFAULT.lastStatus;
	std::string _lastErrorMessage = DEFAULT.lastErrorMessage;

	std::vector<double> _field;       // current flattened grid state
	double _lastTotalMass = 0.0;
	double _lastMaxValue = 0.0;
	double _lastL2Norm = 0.0;
};

#endif /* DIFFUSIONFIELD_H */
