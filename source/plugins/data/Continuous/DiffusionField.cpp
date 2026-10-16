#include "plugins/data/Continuous/DiffusionField.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <cstddef>

#include "kernel/simulator/Event.h"
#include "kernel/simulator/model/Model.h"
#include "kernel/simulator/model/ModelDataManager.h"
#include "kernel/util/Util.h"
#include "tools/Continuous/DiffusionMethodOfLinesSystem.h"
#include "tools/Continuous/OdeSolverFactory.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &DiffusionField::GetPluginInformation;
}
#endif

ModelDataDefinition* DiffusionField::NewInstance(Model* model, std::string name) {
	return new DiffusionField(model, name);
}

DiffusionField::DiffusionField(Model* model, std::string name)
		: ContinuousModelDataDefinition(model, Util::TypeOf<DiffusionField>(), name, 0.1, 0.01, 0u) {
	auto* propDimensions = new SimulationControlUInt(
			std::bind(&DiffusionField::getDimensions, this), std::bind(&DiffusionField::setDimensions, this, std::placeholders::_1),
			Util::TypeOf<DiffusionField>(), getName(), "Dimensions", "");
	auto* propPoints = new SimulationControlUInt(
			std::bind(&DiffusionField::getPointsPerDimension, this), std::bind(&DiffusionField::setPointsPerDimension, this, std::placeholders::_1),
			Util::TypeOf<DiffusionField>(), getName(), "PointsPerDimension", "");
	auto* propLength = new SimulationControlDouble(
			std::bind(&DiffusionField::getDomainLength, this), std::bind(&DiffusionField::setDomainLength, this, std::placeholders::_1),
			Util::TypeOf<DiffusionField>(), getName(), "DomainLength", "");
	auto* propD = new SimulationControlDouble(
			std::bind(&DiffusionField::getDiffusionCoefficient, this), std::bind(&DiffusionField::setDiffusionCoefficient, this, std::placeholders::_1),
			Util::TypeOf<DiffusionField>(), getName(), "DiffusionCoefficient", "");
	auto* propBoundary = new SimulationControlString(
			std::bind(&DiffusionField::getBoundaryCondition, this), std::bind(&DiffusionField::setBoundaryCondition, this, std::placeholders::_1),
			Util::TypeOf<DiffusionField>(), getName(), "BoundaryCondition", "");
	auto* propInitial = new SimulationControlString(
			std::bind(&DiffusionField::getInitialCondition, this), std::bind(&DiffusionField::setInitialCondition, this, std::placeholders::_1),
			Util::TypeOf<DiffusionField>(), getName(), "InitialCondition", "");
	auto* propInitialParam = new SimulationControlDouble(
			std::bind(&DiffusionField::getInitialParameter, this), std::bind(&DiffusionField::setInitialParameter, this, std::placeholders::_1),
			Util::TypeOf<DiffusionField>(), getName(), "InitialParameter", "");
	auto* propOdeSolver = new SimulationControlString(
			std::bind(&DiffusionField::getOdeSolver, this), std::bind(&DiffusionField::setOdeSolver, this, std::placeholders::_1),
			Util::TypeOf<DiffusionField>(), getName(), "OdeSolver", "");


	_parentModel->getControls()->insert(propDimensions);
	_parentModel->getControls()->insert(propPoints);
	_parentModel->getControls()->insert(propLength);
	_parentModel->getControls()->insert(propD);
	_parentModel->getControls()->insert(propBoundary);
	_parentModel->getControls()->insert(propInitial);
	_parentModel->getControls()->insert(propInitialParam);

	_parentModel->getControls()->insert(propOdeSolver);


	_addSimulationControl(propDimensions);
	_addSimulationControl(propPoints);
	_addSimulationControl(propLength);
	_addSimulationControl(propD);
	_addSimulationControl(propBoundary);
	_addSimulationControl(propInitial);
	_addSimulationControl(propInitialParam);

	_addSimulationControl(propOdeSolver);

}

PluginInformation* DiffusionField::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<DiffusionField>(), &DiffusionField::LoadInstance, &DiffusionField::NewInstance);
	info->setCategory("Mathematical/PDE");
	info->setDescriptionHelp("Solves the N-dimensional diffusion equation du/dt = D*Laplacian(u) with the Method of Lines: central differences in space turn it into a system of ODEs, solved in time by your choice of solver (RK4 or Dormand-Prince 5(4)). You can set the number of dimensions, grid size, domain length, diffusion coefficient, boundary condition (Dirichlet or Neumann) and initial condition.");
	return info;
}

ModelDataDefinition* DiffusionField::LoadInstance(Model* model, PersistenceRecord* fields) {
	DiffusionField* newElement = new DiffusionField(model);
	try {
		newElement->_loadInstance(fields);
	} catch (const std::exception& e) {
	}
	return newElement;
}

// --- configuration accessors ---

void DiffusionField::setDimensions(unsigned int dimensions) { _dimensions = dimensions; _field.clear(); }
unsigned int DiffusionField::getDimensions() const { return _dimensions; }
void DiffusionField::setPointsPerDimension(unsigned int pointsPerDimension) { _pointsPerDimension = pointsPerDimension; _field.clear(); }
unsigned int DiffusionField::getPointsPerDimension() const { return _pointsPerDimension; }
void DiffusionField::setDomainLength(double domainLength) { _domainLength = domainLength; _field.clear(); }
double DiffusionField::getDomainLength() const { return _domainLength; }
void DiffusionField::setDiffusionCoefficient(double diffusionCoefficient) { _diffusionCoefficient = diffusionCoefficient; }
double DiffusionField::getDiffusionCoefficient() const { return _diffusionCoefficient; }
void DiffusionField::setBoundaryCondition(std::string boundaryCondition) { _boundaryCondition = boundaryCondition; _field.clear(); }
std::string DiffusionField::getBoundaryCondition() const { return _boundaryCondition; }
void DiffusionField::setInitialCondition(std::string initialCondition) { _initialCondition = initialCondition; _field.clear(); }
std::string DiffusionField::getInitialCondition() const { return _initialCondition; }
void DiffusionField::setInitialParameter(double initialParameter) { _initialParameter = initialParameter; _field.clear(); }
double DiffusionField::getInitialParameter() const { return _initialParameter; }
void DiffusionField::setOdeSolver(std::string odeSolver) {
	_odeSolver = odeSolver.empty() ? OdeSolverFactory::defaultKey() : odeSolver;
}
std::string DiffusionField::getOdeSolver() const { return _odeSolver; }
// --- diagnostics ---

double DiffusionField::getTotalMass() const { return _lastTotalMass; }
double DiffusionField::getMaxValue() const { return _lastMaxValue; }
double DiffusionField::getL2Norm() const { return _lastL2Norm; }
const std::vector<double>& DiffusionField::getField() const { return _field; }

double DiffusionField::getFieldValue(const std::vector<unsigned int>& multiIndex) const {
	if (multiIndex.size() != _dimensions || _field.empty()) {
		return 0.0;
	}
	std::size_t idx = 0;
	// Row-major: turn the coordinates back into a single flat index.
	std::vector<std::size_t> strides(_dimensions, 1);
	for (std::size_t d = _dimensions; d-- > 1;) {
		strides[d - 1] = strides[d] * _pointsPerDimension;
	}
	for (std::size_t d = 0; d < _dimensions; ++d) {
		if (multiIndex[d] >= _pointsPerDimension) return 0.0;
		idx += static_cast<std::size_t>(multiIndex[d]) * strides[d];
	}
	return idx < _field.size() ? _field[idx] : 0.0;
}

// --- display ---

std::string DiffusionField::showFieldAsAscii() const {
	if (_dimensions != 2u || _field.empty()) {
		return "";
	}

	const std::string shades = " .:-=+*#%@";
	const double maxv = getMaxValue();

	std::string out;
	out += "DiffusionField \"" + getName() + "\"";
	out += " | t=" + Util::StrTruncIfInt(std::to_string(_currentTime));
	out += " | mass=" + Util::StrTruncIfInt(std::to_string(_lastTotalMass));
	out += " | max=" + Util::StrTruncIfInt(std::to_string(_lastMaxValue));
	out += "\n\n";

	for (unsigned int i = 0; i < _pointsPerDimension; ++i) {
		for (unsigned int j = 0; j < _pointsPerDimension; ++j) {
			const double v = getFieldValue({i, j});

			std::size_t k = 0;
			if (maxv > 0.0) {
				const double x = std::clamp(v / maxv, 0.0, 1.0);
				k = static_cast<std::size_t>(x * static_cast<double>(shades.size() - 1));
			}

			out += shades[k];
			out += shades[k];
		}
		out += '\n';
	}

	return out;
}

// --- building the discretized system ---

bool DiffusionField::buildSystem(DiffusionMethodOfLinesSystem* system, std::string& errorMessage) const {
	if (system == nullptr) {
		errorMessage += "DiffusionField \"" + getName() + "\" received a null system. ";
		return false;
	}
	if (_dimensions == 0u) {
		errorMessage += "DiffusionField \"" + getName() + "\" must define Dimensions >= 1. ";
		return false;
	}
	if (_pointsPerDimension < 3u) {
		errorMessage += "DiffusionField \"" + getName() + "\" must define PointsPerDimension >= 3. ";
		return false;
	}
	if (_domainLength <= 0.0) {
		errorMessage += "DiffusionField \"" + getName() + "\" must define DomainLength > 0. ";
		return false;
	}
	const double h = _domainLength / static_cast<double>(_pointsPerDimension - 1u);
	std::vector<unsigned int> shape(_dimensions, _pointsPerDimension);
	std::vector<double> spacing(_dimensions, h);
	const DiffusionMethodOfLinesSystem::Boundary boundary =
			(_boundaryCondition == "Neumann") ? DiffusionMethodOfLinesSystem::Boundary::Neumann
			                                  : DiffusionMethodOfLinesSystem::Boundary::Dirichlet;
	*system = DiffusionMethodOfLinesSystem(shape, spacing, _diffusionCoefficient, boundary);
	if (!system->isValid()) {
		errorMessage += "DiffusionField \"" + getName() + "\" produced an invalid discretization. ";
		return false;
	}
	return true;
}

void DiffusionField::buildInitialField(const DiffusionMethodOfLinesSystem& system, std::vector<double>& field) const {
	field.assign(system.totalNodes(), 0.0);
	if (_initialCondition == "Gaussian") {
		const double sigma = (_initialParameter > 0.0) ? _initialParameter : 0.1;
		system.fillGaussian(1.0, sigma, field.data());
	} else if (_initialCondition == "Constant") {
		std::fill(field.begin(), field.end(), _initialParameter);
	} else { // "SineMode" (default)
		const unsigned int mode = (_initialParameter >= 1.0) ? static_cast<unsigned int>(_initialParameter) : 1u;
		std::vector<unsigned int> modes(_dimensions, mode);
		system.fillSineModes(modes, 1.0, field.data());
	}
}

void DiffusionField::refreshDiagnostics(const DiffusionMethodOfLinesSystem& system) {
	if (_field.empty()) {
		_lastTotalMass = _lastMaxValue = _lastL2Norm = 0.0;
		return;
	}
	_lastTotalMass = system.totalMass(_field.data());
	_lastMaxValue = system.maxValue(_field.data());
	_lastL2Norm = system.l2Norm(_field.data());
}

// --- model-specific evolution ---

void DiffusionField::initializeContinuousState() {
    _field.clear();
    _lastTotalMass = _lastMaxValue = _lastL2Norm = 0.0;
}

bool DiffusionField::evolveState(double currentTime, double dt, std::string& errorMessage) {
    DiffusionMethodOfLinesSystem system;
    if (!buildSystem(&system, errorMessage)) return false;
    if (_field.empty()) {
        buildInitialField(system, _field);
        refreshDiagnostics(system);
    }
    std::vector<double> next(_field.size(), 0.0);
    std::unique_ptr<OdeSolver_if> solver = OdeSolverFactory::create(_odeSolver);
    if (!solver || !solver->advance(system, currentTime, dt, _field.data(), next.data())) {
        errorMessage = "DiffusionField \"" + getName() + "\" failed to advance with solver \"" + _odeSolver + "\".";
        return false;
    }
    _field.swap(next);
    refreshDiagnostics(system);
    return true;
}

void DiffusionField::onContinuousRunCompleted() {
    // A zero-duration run still exposes the initial field and its diagnostics.
    if (_field.empty()) {
        DiffusionMethodOfLinesSystem system;
        std::string errorMessage;
        if (buildSystem(&system, errorMessage)) {
            buildInitialField(system, _field);
            refreshDiagnostics(system);
        } else {
            _lastStatus = "Failed";
            _lastErrorMessage = errorMessage;
        }
    }
}

// --- ModelDataDefinition overrides ---

std::string DiffusionField::show() {
	return ModelDataDefinition::show() +
			",dimensions=" + std::to_string(_dimensions) +
			",pointsPerDimension=" + std::to_string(_pointsPerDimension) +
			",domainLength=" + Util::StrTruncIfInt(std::to_string(_domainLength)) +
			",diffusionCoefficient=" + Util::StrTruncIfInt(std::to_string(_diffusionCoefficient)) +
			",boundaryCondition=\"" + _boundaryCondition + "\"" +
			",initialCondition=\"" + _initialCondition + "\"" +
			",initialParameter=" + Util::StrTruncIfInt(std::to_string(_initialParameter)) +
			",stopTime=" + Util::StrTruncIfInt(std::to_string(_stopTime)) +
			",stepSize=" + Util::StrTruncIfInt(std::to_string(_stepSize)) +
			",stepsToSynchronize=" + std::to_string(getStepsToSynchronize()) +
			",odeSolver=\"" + _odeSolver + "\"" +
			",lastStatus=\"" + _lastStatus + "\"";
}

bool DiffusionField::_loadInstance(PersistenceRecord* fields) {
	bool res = ContinuousModelDataDefinition::_loadInstance(fields);
	if (res) {
		_dimensions = fields->loadField("dimensions", DEFAULT.dimensions);
		_pointsPerDimension = fields->loadField("pointsPerDimension", DEFAULT.pointsPerDimension);
		_domainLength = fields->loadField("domainLength", DEFAULT.domainLength);
		_diffusionCoefficient = fields->loadField("diffusionCoefficient", DEFAULT.diffusionCoefficient);
		_boundaryCondition = fields->loadField("boundaryCondition", DEFAULT.boundaryCondition);
		_initialCondition = fields->loadField("initialCondition", DEFAULT.initialCondition);
		_initialParameter = fields->loadField("initialParameter", DEFAULT.initialParameter);
		setStepsToSynchronize(fields->loadField("stepsToSynchronize",
			fields->loadField("autoSchedule", 0u) != 0u ? 1u : 0u));
		_odeSolver = fields->loadField("odeSolver", DEFAULT.odeSolver);
		if (_odeSolver.empty()) {
			_odeSolver = OdeSolverFactory::defaultKey();
		}
		_field.clear();
	}
	return res;
}

void DiffusionField::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
	ContinuousModelDataDefinition::_saveInstance(fields, saveDefaultValues);
	fields->saveField("dimensions", _dimensions, DEFAULT.dimensions, saveDefaultValues);
	fields->saveField("pointsPerDimension", _pointsPerDimension, DEFAULT.pointsPerDimension, saveDefaultValues);
	fields->saveField("domainLength", _domainLength, DEFAULT.domainLength, saveDefaultValues);
	fields->saveField("diffusionCoefficient", _diffusionCoefficient, DEFAULT.diffusionCoefficient, saveDefaultValues);
	fields->saveField("boundaryCondition", _boundaryCondition, DEFAULT.boundaryCondition, saveDefaultValues);
	fields->saveField("initialCondition", _initialCondition, DEFAULT.initialCondition, saveDefaultValues);
	fields->saveField("initialParameter", _initialParameter, DEFAULT.initialParameter, saveDefaultValues);
	fields->saveField("odeSolver", _odeSolver, DEFAULT.odeSolver, saveDefaultValues);
}

bool DiffusionField::_check(std::string& errorMessage) {
	bool resultAll = ContinuousModelDataDefinition::_check(errorMessage);
	if (getName().empty()) {
		errorMessage += "DiffusionField must define a non-empty name. ";
		resultAll = false;
	}
	if (_dimensions == 0u) {
		errorMessage += "DiffusionField \"" + getName() + "\" must define Dimensions >= 1. ";
		resultAll = false;
	}
	if (_pointsPerDimension < 3u) {
		errorMessage += "DiffusionField \"" + getName() + "\" must define PointsPerDimension >= 3. ";
		resultAll = false;
	}
	if (_domainLength <= 0.0) {
		errorMessage += "DiffusionField \"" + getName() + "\" must define DomainLength > 0. ";
		resultAll = false;
	}
	const std::size_t maxBytes = 512ull * 1024ull * 1024ull;
	std::size_t totalNodes = 1;

	// Only meaningful (and safe to divide by) when the points check above passed.
	if (_pointsPerDimension >= 3u) {
		for (unsigned int d = 0; d < _dimensions; ++d) {
			if (totalNodes > (maxBytes / sizeof(double)) / _pointsPerDimension) {
				errorMessage += "DiffusionField \"" + getName() +
								"\" grid is too large. Reduce Dimensions or PointsPerDimension. ";
				resultAll = false;
				break;
			}
			totalNodes *= _pointsPerDimension;
		}
	}
	if (_diffusionCoefficient < 0.0) {
		errorMessage += "DiffusionField \"" + getName() + "\" must define DiffusionCoefficient >= 0. ";
		resultAll = false;
	}
	if (_boundaryCondition != "Dirichlet" && _boundaryCondition != "Neumann") {
		errorMessage += "DiffusionField \"" + getName() + "\" has unknown BoundaryCondition \"" + _boundaryCondition + "\" (use Dirichlet or Neumann). ";
		resultAll = false;
	}
	if (_initialCondition != "SineMode" && _initialCondition != "Gaussian" && _initialCondition != "Constant") {
		errorMessage += "DiffusionField \"" + getName() + "\" has unknown InitialCondition \"" + _initialCondition + "\" (use SineMode, Gaussian or Constant). ";
		resultAll = false;
	}
	if (!OdeSolverFactory::isRegistered(_odeSolver)) {
		errorMessage += "DiffusionField \"" + getName() + "\" references unknown ODE solver \"" + _odeSolver + "\". ";
		resultAll = false;
	}
	// Heads-up (not enforced): with the fixed RK4, the step is only stable if dt
	// is roughly below h^2 / (2*N*D). Above that the solution can blow up - the
	// adaptive solver handles it by taking smaller sub-steps.
	return resultAll;
}

