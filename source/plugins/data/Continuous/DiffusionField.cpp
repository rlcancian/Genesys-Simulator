#include "plugins/data/Continuous/DiffusionField.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <cstddef>

#include "kernel/simulator/model/Model.h"
#include "kernel/simulator/model/ModelDataManager.h"
#include "kernel/util/Util.h"
#include "tools/DiffusionMethodOfLinesSystem.h"
#include "tools/OdeSolverFactory.h"

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &DiffusionField::GetPluginInformation;
}
#endif

ModelDataDefinition* DiffusionField::NewInstance(Model* model, std::string name) {
	return new DiffusionField(model, name);
}

DiffusionField::DiffusionField(Model* model, std::string name)
		: ModelDataDefinition(model, Util::TypeOf<DiffusionField>(), name) {
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
	auto* propStartTime = new SimulationControlDouble(
			std::bind(&DiffusionField::getStartTime, this), std::bind(&DiffusionField::setStartTime, this, std::placeholders::_1),
			Util::TypeOf<DiffusionField>(), getName(), "StartTime", "");
	auto* propStopTime = new SimulationControlDouble(
			std::bind(&DiffusionField::getStopTime, this), std::bind(&DiffusionField::setStopTime, this, std::placeholders::_1),
			Util::TypeOf<DiffusionField>(), getName(), "StopTime", "");
	auto* propStepSize = new SimulationControlDouble(
			std::bind(&DiffusionField::getStepSize, this), std::bind(&DiffusionField::setStepSize, this, std::placeholders::_1),
			Util::TypeOf<DiffusionField>(), getName(), "StepSize", "");
	auto* propCurrentTime = new SimulationControlDouble(
			std::bind(&DiffusionField::getCurrentTime, this), std::bind(&DiffusionField::setCurrentTime, this, std::placeholders::_1),
			Util::TypeOf<DiffusionField>(), getName(), "CurrentTime", "");
	auto* propOdeSolver = new SimulationControlString(
			std::bind(&DiffusionField::getOdeSolver, this), std::bind(&DiffusionField::setOdeSolver, this, std::placeholders::_1),
			Util::TypeOf<DiffusionField>(), getName(), "OdeSolver", "");
	auto* propLastStatus = new SimulationControlString(
			std::bind(&DiffusionField::getLastStatus, this), std::bind(&DiffusionField::setLastStatus, this, std::placeholders::_1),
			Util::TypeOf<DiffusionField>(), getName(), "LastStatus", "");
	auto* propLastErrorMessage = new SimulationControlString(
			std::bind(&DiffusionField::getLastErrorMessage, this), std::bind(&DiffusionField::setLastErrorMessage, this, std::placeholders::_1),
			Util::TypeOf<DiffusionField>(), getName(), "LastErrorMessage", "");

	_parentModel->getControls()->insert(propDimensions);
	_parentModel->getControls()->insert(propPoints);
	_parentModel->getControls()->insert(propLength);
	_parentModel->getControls()->insert(propD);
	_parentModel->getControls()->insert(propBoundary);
	_parentModel->getControls()->insert(propInitial);
	_parentModel->getControls()->insert(propInitialParam);
	_parentModel->getControls()->insert(propStartTime);
	_parentModel->getControls()->insert(propStopTime);
	_parentModel->getControls()->insert(propStepSize);
	_parentModel->getControls()->insert(propCurrentTime);
	_parentModel->getControls()->insert(propOdeSolver);
	_parentModel->getControls()->insert(propLastStatus);
	_parentModel->getControls()->insert(propLastErrorMessage);

	_addSimulationControl(propDimensions);
	_addSimulationControl(propPoints);
	_addSimulationControl(propLength);
	_addSimulationControl(propD);
	_addSimulationControl(propBoundary);
	_addSimulationControl(propInitial);
	_addSimulationControl(propInitialParam);
	_addSimulationControl(propStartTime);
	_addSimulationControl(propStopTime);
	_addSimulationControl(propStepSize);
	_addSimulationControl(propCurrentTime);
	_addSimulationControl(propOdeSolver);
	_addSimulationControl(propLastStatus);
	_addSimulationControl(propLastErrorMessage);
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
void DiffusionField::setStartTime(double startTime) { _startTime = startTime; }
double DiffusionField::getStartTime() const { return _startTime; }
void DiffusionField::setStopTime(double stopTime) { _stopTime = stopTime; }
double DiffusionField::getStopTime() const { return _stopTime; }
void DiffusionField::setStepSize(double stepSize) { _stepSize = stepSize; }
double DiffusionField::getStepSize() const { return _stepSize; }
void DiffusionField::setCurrentTime(double currentTime) { _currentTime = currentTime; }
double DiffusionField::getCurrentTime() const { return _currentTime; }
void DiffusionField::setOdeSolver(std::string odeSolver) {
	_odeSolver = odeSolver.empty() ? OdeSolverFactory::defaultKey() : odeSolver;
}
std::string DiffusionField::getOdeSolver() const { return _odeSolver; }
void DiffusionField::setLastStatus(std::string lastStatus) { _lastStatus = lastStatus; }
std::string DiffusionField::getLastStatus() const { return _lastStatus; }
void DiffusionField::setLastErrorMessage(std::string lastErrorMessage) { _lastErrorMessage = lastErrorMessage; }
std::string DiffusionField::getLastErrorMessage() const { return _lastErrorMessage; }

// --- diagnostics ---

double DiffusionField::getTotalMass() const { return _lastTotalMass; }
double DiffusionField::getMaxValue() const { return _lastMaxValue; }
double DiffusionField::getL2Norm() const { return _lastL2Norm; }
const std::vector<double>& DiffusionField::getField() const { return _field; }

double DiffusionField::getFieldValue(const std::vector<unsigned int>& multiIndex) const {
	if (multiIndex.size() != _dimensions || _field.empty()) {
		return 0.0;
	}
	std::size_t stride = 1, idx = 0;
	// Row-major: turn the coordinates back into a single flat index.
	std::vector<std::size_t> strides(_dimensions, 1);
	for (std::size_t d = _dimensions; d-- > 1;) {
		strides[d - 1] = strides[d] * _pointsPerDimension;
	}
	for (std::size_t d = 0; d < _dimensions; ++d) {
		if (multiIndex[d] >= _pointsPerDimension) return 0.0;
		idx += static_cast<std::size_t>(multiIndex[d]) * strides[d];
	}
	(void) stride;
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

// --- execution ---

bool DiffusionField::simulate(std::string& errorMessage) {
	return simulate(_startTime, _stopTime, _stepSize, errorMessage);
}

bool DiffusionField::simulate(double startTime, double stopTime, double stepSize, std::string& errorMessage) {
	_startTime = startTime;
	_stopTime = stopTime;
	_stepSize = stepSize;
	_currentTime = _startTime;
	_field.clear();
	_lastStatus = "Running";
	_lastErrorMessage.clear();

	if (!_check(errorMessage)) {
		_lastStatus = "Failed";
		_lastErrorMessage = errorMessage;
		return false;
	}
	while (_currentTime < _stopTime) {
		if (!advanceOneStep(errorMessage)) {
			return false;
		}
	}
	_lastStatus = "Completed";
	return true;
}

bool DiffusionField::advanceOneStep(std::string& errorMessage) {
	errorMessage.clear();
	if (_stepSize <= 0.0) {
		_lastStatus = "Failed";
		_lastErrorMessage = "DiffusionField \"" + getName() + "\" must define StepSize > 0.";
		errorMessage = _lastErrorMessage;
		return false;
	}
	if (_currentTime >= _stopTime) {
		_lastStatus = "Completed";
		return true;
	}

	DiffusionMethodOfLinesSystem system;
	if (!buildSystem(&system, errorMessage)) {
		_lastStatus = "Failed";
		_lastErrorMessage = errorMessage;
		return false;
	}

	// Set up the field on the first step of a run.
	if (_field.empty()) {
		buildInitialField(system, _field);
		_currentTime = _startTime;
		refreshDiagnostics(system);
	}

	const double dt = std::min(_stepSize, _stopTime - _currentTime);
	std::vector<double> next(_field.size(), 0.0);

	// Get the chosen solver from the factory; we only use it through OdeSolver_if.
	std::unique_ptr<OdeSolver_if> solver = OdeSolverFactory::create(_odeSolver);
	if (!solver || !solver->advance(system, _currentTime, dt, _field.data(), next.data())) {
		_lastStatus = "Failed";
		_lastErrorMessage = "DiffusionField \"" + getName() + "\" failed to advance the ODE system with solver \"" + _odeSolver + "\".";
		errorMessage = _lastErrorMessage;
		return false;
	}

	_field.swap(next);
	_currentTime += dt;
	refreshDiagnostics(system);
	_lastStatus = _currentTime >= _stopTime ? "Completed" : "Running";
	return true;
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
			",startTime=" + Util::StrTruncIfInt(std::to_string(_startTime)) +
			",stopTime=" + Util::StrTruncIfInt(std::to_string(_stopTime)) +
			",stepSize=" + Util::StrTruncIfInt(std::to_string(_stepSize)) +
			",odeSolver=\"" + _odeSolver + "\"" +
			",lastStatus=\"" + _lastStatus + "\"";
}

bool DiffusionField::_loadInstance(PersistenceRecord* fields) {
	bool res = ModelDataDefinition::_loadInstance(fields);
	if (res) {
		_dimensions = fields->loadField("dimensions", DEFAULT.dimensions);
		_pointsPerDimension = fields->loadField("pointsPerDimension", DEFAULT.pointsPerDimension);
		_domainLength = fields->loadField("domainLength", DEFAULT.domainLength);
		_diffusionCoefficient = fields->loadField("diffusionCoefficient", DEFAULT.diffusionCoefficient);
		_boundaryCondition = fields->loadField("boundaryCondition", DEFAULT.boundaryCondition);
		_initialCondition = fields->loadField("initialCondition", DEFAULT.initialCondition);
		_initialParameter = fields->loadField("initialParameter", DEFAULT.initialParameter);
		_startTime = fields->loadField("startTime", DEFAULT.startTime);
		_stopTime = fields->loadField("stopTime", DEFAULT.stopTime);
		_stepSize = fields->loadField("stepSize", DEFAULT.stepSize);
		_currentTime = fields->loadField("currentTime", _startTime);
		_odeSolver = fields->loadField("odeSolver", DEFAULT.odeSolver);
		if (_odeSolver.empty()) {
			_odeSolver = OdeSolverFactory::defaultKey();
		}
		_lastStatus = fields->loadField("lastStatus", DEFAULT.lastStatus);
		_lastErrorMessage = fields->loadField("lastErrorMessage", DEFAULT.lastErrorMessage);
		_field.clear();
	}
	return res;
}

void DiffusionField::_saveInstance(PersistenceRecord* fields, bool saveDefaultValues) {
	ModelDataDefinition::_saveInstance(fields, saveDefaultValues);
	fields->saveField("dimensions", _dimensions, DEFAULT.dimensions, saveDefaultValues);
	fields->saveField("pointsPerDimension", _pointsPerDimension, DEFAULT.pointsPerDimension, saveDefaultValues);
	fields->saveField("domainLength", _domainLength, DEFAULT.domainLength, saveDefaultValues);
	fields->saveField("diffusionCoefficient", _diffusionCoefficient, DEFAULT.diffusionCoefficient, saveDefaultValues);
	fields->saveField("boundaryCondition", _boundaryCondition, DEFAULT.boundaryCondition, saveDefaultValues);
	fields->saveField("initialCondition", _initialCondition, DEFAULT.initialCondition, saveDefaultValues);
	fields->saveField("initialParameter", _initialParameter, DEFAULT.initialParameter, saveDefaultValues);
	fields->saveField("startTime", _startTime, DEFAULT.startTime, saveDefaultValues);
	fields->saveField("stopTime", _stopTime, DEFAULT.stopTime, saveDefaultValues);
	fields->saveField("stepSize", _stepSize, DEFAULT.stepSize, saveDefaultValues);
	fields->saveField("currentTime", _currentTime, DEFAULT.currentTime, saveDefaultValues);
	fields->saveField("odeSolver", _odeSolver, DEFAULT.odeSolver, saveDefaultValues);
	fields->saveField("lastStatus", _lastStatus, DEFAULT.lastStatus, saveDefaultValues);
	fields->saveField("lastErrorMessage", _lastErrorMessage, DEFAULT.lastErrorMessage, saveDefaultValues);
}

bool DiffusionField::_check(std::string& errorMessage) {
	bool resultAll = true;
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

	for (unsigned int d = 0; d < _dimensions; ++d) {
		if (totalNodes > (maxBytes / sizeof(double)) / _pointsPerDimension) {
			errorMessage += "DiffusionField \"" + getName() +
							"\" grid is too large. Reduce Dimensions or PointsPerDimension. ";
			resultAll = false;
			break;
		}
		totalNodes *= _pointsPerDimension;
	}
	if (_diffusionCoefficient < 0.0) {
		errorMessage += "DiffusionField \"" + getName() + "\" must define DiffusionCoefficient >= 0. ";
		resultAll = false;
	}
	if (_stepSize <= 0.0) {
		errorMessage += "DiffusionField \"" + getName() + "\" must define StepSize > 0. ";
		resultAll = false;
	}
	if (_stopTime < _startTime) {
		errorMessage += "DiffusionField \"" + getName() + "\" must define StopTime >= StartTime. ";
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

void DiffusionField::_initBetweenReplications() {
	_currentTime = _startTime;
	_field.clear();
	_lastTotalMass = _lastMaxValue = _lastL2Norm = 0.0;
	_lastStatus = "Idle";
	_lastErrorMessage.clear();
}
