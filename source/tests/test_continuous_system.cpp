/* Controlled evaluation test for ContinuousSystemComponent without kernel test helper. */

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "kernel/simulator/EntityType.h"
#include "kernel/simulator/Model.h"
#include "kernel/simulator/PluginManager.h"
#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/TraceManager.h"
#include "plugins/components/Continuous/ContinuousSystemComponent.h"
#include "plugins/components/Logic/Create.h"
#include "plugins/components/Logic/Dispose.h"
#include "plugins/data/Continuous/ODESolver.h"
#include "plugins/data/Logic/Variable.h"

struct Sample {
	double t;
	double x;
	double v;
	double energy;
};

class RecordingContinuousSystemComponent : public ContinuousSystemComponent {
public:
	RecordingContinuousSystemComponent(Model* model, ODESolver* solver, std::vector<Sample>* samples, std::string name = "")
		: ContinuousSystemComponent(model, name), _solver(solver), _samples(samples) {
		setOdeSolver(solver);
	}
protected:
	void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override {
		ContinuousSystemComponent::_onDispatchEvent(entity, inputPortNumber);
		const double t = _solver->getCurrentTime();
		const double x = _solver->getStateValue(0);
		const double v = _solver->getStateValue(1);
		_samples->push_back({t, x, v, x * x + v * v});
	}
private:
	ODESolver* _solver;
	std::vector<Sample>* _samples;
};

namespace {
bool insertPluginOrFail(PluginManager* pluginManager, const std::string& filename) {
	Plugin* plugin = pluginManager->insert(filename);
	if (plugin == nullptr) {
		std::cerr << "Plugin insert failed: " << filename << std::endl;
		return false;
	}
	return true;
}
}

int main() {
	const double twoPi = 2.0 * std::acos(-1.0);
	const unsigned int intervals = 64;
	const double eventInterval = twoPi / intervals;
	std::ostringstream eventIntervalExpression;
	eventIntervalExpression << std::setprecision(17) << eventInterval;
	std::vector<Sample> samples;

	Simulator* simulator = new Simulator();
	Model* model = simulator->getModelManager()->newModel();
	model->getTracer()->setTraceLevel(TraceManager::Level::L3_errorRecover);

	PluginManager* pluginManager = simulator->getPluginManager();
	if (!insertPluginOrFail(pluginManager, "create.so") ||
	    !insertPluginOrFail(pluginManager, "dispose.so") ||
	    !insertPluginOrFail(pluginManager, "continuoussystemcomponent.so") ||
	    !insertPluginOrFail(pluginManager, "odesolver.so") ||
	    !insertPluginOrFail(pluginManager, "variable.so")) {
		return 1;
	}

	ODESolver* odeSolver = new ODESolver(model, "HarmonicOscillator");
	odeSolver->setTimeVariableName("t");
	odeSolver->setStateVariableNames({"x", "v"});
	odeSolver->setEquationExpressions({"v", "-x"});
	odeSolver->setStep(0.01);
	odeSolver->setStateValues({1.0, 0.0});
	odeSolver->setInitialStateValues({1.0, 0.0});
	model->getDataManager()->insert(odeSolver);

	EntityType* entityType = pluginManager->newInstance<EntityType>(model, "entitytype");
	if (entityType == nullptr) {
		entityType = new EntityType(model, "entitytype");
		model->getDataManager()->insert(entityType);
	}

	Create* create = pluginManager->newInstance<Create>(model, "Create_1");
	Dispose* dispose = pluginManager->newInstance<Dispose>(model, "Dispose_1");
	if (create == nullptr || dispose == nullptr) {
		std::cerr << "Create/Dispose instantiation failed" << std::endl;
		return 1;
	}
	create->setTimeBetweenCreationsExpression(eventIntervalExpression.str());
	create->setEntityType(entityType);
	create->setMaxCreations(intervals + 1);

	RecordingContinuousSystemComponent* continuous =
			new RecordingContinuousSystemComponent(model, odeSolver, &samples, "ContinuousSystem_1");
	create->connectTo(continuous);
	continuous->connectTo(dispose);

	model->getSimulation()->setReplicationLength(twoPi + 1e-9);
	model->getSimulation()->setNumberOfReplications(1);

	if (!model->check()) {
		std::cerr << "Model check failed" << std::endl;
		return 1;
	}

	try {
		model->getSimulation()->start();
	} catch (const std::exception& e) {
		std::cerr << "Simulation failed: " << e.what() << std::endl;
		return 1;
	}

	if (samples.empty()) {
		std::cerr << "No samples collected" << std::endl;
		return 1;
	}

	const Sample& first = samples.front();
	const Sample& last = samples.back();
	double minEnergy = first.energy;
	double maxEnergy = first.energy;
	for (const Sample& sample : samples) {
		minEnergy = std::min(minEnergy, sample.energy);
		maxEnergy = std::max(maxEnergy, sample.energy);
	}
	const double errX = std::abs(last.x - std::cos(twoPi));
	const double errV = std::abs(last.v + std::sin(twoPi));
	const double maxEnergyVariationPercent = (maxEnergy - minEnergy) / first.energy * 100.0;

	std::cout << std::setprecision(12)
	          << "x_final=" << last.x << " v_final=" << last.v
	          << " err_x=" << errX << " err_v=" << errV
	          << " max_energy_var=" << maxEnergyVariationPercent << "%" << std::endl;

	return (errX < 1e-4 && errV < 1e-4 && maxEnergyVariationPercent < 1.0) ? 0 : 1;
}
