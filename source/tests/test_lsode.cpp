/* Controlled evaluation test for LSODE without kernel test helper. */

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
#include "plugins/components/Continuous/LSODE.h"
#include "plugins/components/Logic/Create.h"
#include "plugins/components/Logic/Dispose.h"
#include "plugins/data/Logic/Variable.h"

struct LSODESample {
	double t;
	double x;
	double v;
	double energy;
};

struct LSODEInitialState {
	Model* model;
	Variable* timeVariable;
	Variable* stateVariable;
	void OnReplicationStart(SimulationEvent*) {
		model->getDataManager()->insert(stateVariable);
		model->getDataManager()->insert(timeVariable);
		timeVariable->setValue(0.0);
		stateVariable->setValue(1.0, "0");
		stateVariable->setValue(0.0, "1");
	}
};

class RecordingLSODE : public LSODE {
public:
	RecordingLSODE(Model* model, Variable* timeVariable, Variable* stateVariable,
			std::vector<LSODESample>* samples, std::string name = "")
			: LSODE(model, name), _timeVariable(timeVariable), _stateVariable(stateVariable), _samples(samples) {
	}
protected:
	void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override {
		LSODE::_onDispatchEvent(entity, inputPortNumber);
		const double t = _timeVariable->getValue();
		const double x = _stateVariable->getValue("0");
		const double v = _stateVariable->getValue("1");
		_samples->push_back({t, x, v, x * x + v * v});
	}
private:
	Variable* _timeVariable;
	Variable* _stateVariable;
	std::vector<LSODESample>* _samples;
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
	std::vector<LSODESample> samples;

	Simulator* simulator = new Simulator();
	Model* model = simulator->getModelManager()->newModel();
	model->getTracer()->setTraceLevel(TraceManager::Level::L3_errorRecover);

	PluginManager* pluginManager = simulator->getPluginManager();
	if (!insertPluginOrFail(pluginManager, "create.so") ||
	    !insertPluginOrFail(pluginManager, "dispose.so") ||
	    !insertPluginOrFail(pluginManager, "lsode.so") ||
	    !insertPluginOrFail(pluginManager, "variable.so")) {
		return 1;
	}

	Variable* state = pluginManager->newInstance<Variable>(model, "LSODEVar");
	Variable* time = pluginManager->newInstance<Variable>(model, "t");
	if (state == nullptr || time == nullptr) {
		std::cerr << "Variable instantiation failed" << std::endl;
		return 1;
	}
	state->insertDimentionSize(2);
	model->getDataManager()->insert(state);
	model->getDataManager()->insert(time);

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

	RecordingLSODE* lsode = new RecordingLSODE(model, time, state, &samples, "LSODE_1");
	lsode->setTimeVariable(time);
	lsode->setVariable(state);
	lsode->setStep(0.01);
	lsode->addDiffEquation("LSODEVar[1]");
	lsode->addDiffEquation("-LSODEVar[0]");

	create->connectTo(lsode);
	lsode->connectTo(dispose);

	model->getSimulation()->setReplicationLength(twoPi + 1e-9);
	model->getSimulation()->setNumberOfReplications(1);

	if (!model->check()) {
		std::cerr << "Model check failed" << std::endl;
		return 1;
	}

	model->getDataManager()->insert(state);
	model->getDataManager()->insert(time);
	time->setValue(0.0);
	state->setValue(1.0, "0");
	state->setValue(0.0, "1");

	LSODEInitialState initialState{model, time, state};
	model->getOnEventManager()->addOnReplicationStartHandler(&initialState, &LSODEInitialState::OnReplicationStart);

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

	const LSODESample& first = samples.front();
	const LSODESample& last = samples.back();
	double minEnergy = first.energy;
	double maxEnergy = first.energy;
	for (const LSODESample& sample : samples) {
		minEnergy = std::min(minEnergy, sample.energy);
		maxEnergy = std::max(maxEnergy, sample.energy);
	}
	const double errX = std::abs(last.x - std::cos(last.t));
	const double errV = std::abs(last.v + std::sin(last.t));
	const double maxEnergyVariationPercent = (maxEnergy - minEnergy) / first.energy * 100.0;

	std::cout << std::fixed << std::setprecision(10)
	          << "t=" << last.t << " x=" << last.x << " v=" << last.v
	          << " err_x=" << errX << " err_v=" << errV
	          << " max_energy_var=" << maxEnergyVariationPercent << "%" << std::endl;

	return (errX < 1e-4 && errV < 1e-4 && maxEnergyVariationPercent < 1.0) ? 0 : 1;
}
