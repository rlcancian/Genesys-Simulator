/*
 * Teste para LSODE com oscilador harmonico.
 *
 * Sistema: dx/dt = v, dv/dt = -x
 * Solucao exata: x(t) = cos(t), v(t) = -sin(t)
 */

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

#include "kernel/simulator/essentialPlugins/EntityType.h"
#include "kernel/simulator/model/Model.h"
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
			: LSODE(model, name), _timeVariable(timeVariable), _stateVariable(stateVariable),
			  _samples(samples) {
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

int main() {
	std::cout << "=== Teste LSODE - Oscilador Harmonico ===" << std::endl;
	std::cout << std::endl;

	const double twoPi = 2.0 * std::acos(-1.0);
	const unsigned int intervals = 64;
	const double eventInterval = twoPi / intervals;
	const double replicationLength = twoPi + 1e-9;
	std::ostringstream eventIntervalExpression;
	eventIntervalExpression << std::setprecision(17) << eventInterval;
	std::vector<LSODESample> samples;

	Simulator* simulator = new Simulator();
	Model* model = simulator->getModelManager()->newModel();
	model->getTracer()->setTraceLevel(TraceManager::Level::L3_errorRecover);

	// EntityType já vem registrado por padrão em _insertDefaultKernelElements
	PluginManager* pluginManager = simulator->getPluginManager();
	if (pluginManager->insert("create.so") == nullptr ||
	    pluginManager->insert("dispose.so") == nullptr ||
	    pluginManager->insert("lsode.so") == nullptr ||
	    pluginManager->insert("variable.so") == nullptr) {
		std::cerr << "Erro ao carregar plugins necessarios" << std::endl;
		return 1;
	}

	Variable* state = pluginManager->newInstance<Variable>(model, "LSODEVar");
	state->insertDimentionSize(2);
	model->getDataManager()->insert(state);

	Variable* time = pluginManager->newInstance<Variable>(model, "t");
	model->getDataManager()->insert(time);

	EntityType* entityType = pluginManager->newInstance<EntityType>(model, "entitytype");

	Create* create = pluginManager->newInstance<Create>(model, "Create_1");
	create->setTimeBetweenCreationsExpression(eventIntervalExpression.str());
	create->setEntityType(entityType);
	create->setMaxCreations(intervals + 1);

	RecordingLSODE* lsode = new RecordingLSODE(model, time, state, &samples, "LSODE_1");
	lsode->setTimeVariable(time);
	lsode->setVariable(state);
	lsode->setStep(0.01);
	lsode->addDiffEquation("LSODEVar[1]");
	lsode->addDiffEquation("-LSODEVar[0]");

	Dispose* dispose = pluginManager->newInstance<Dispose>(model, "Dispose_1");

	create->connectTo(lsode);
	lsode->connectTo(dispose);

	model->getSimulation()->setReplicationLength(replicationLength);
	model->getSimulation()->setNumberOfReplications(1);

	if (!model->check()) {
		std::cerr << "Erro ao verificar modelo" << std::endl;
		return 1;
	}

	model->getDataManager()->insert(state);
	model->getDataManager()->insert(time);
	time->setValue(0.0);
	state->setValue(1.0, "0");
	state->setValue(0.0, "1");

	if (model->getDataManager()->getDataDefinition(Util::TypeOf<Variable>(), "LSODEVar") != state) {
		std::cerr << "Erro: variavel LSODEVar nao esta registrada no DataManager como Variable" << std::endl;
		return 1;
	}

	bool parseSuccessX = false;
	bool parseSuccessV = false;
	std::string parseErrorX;
	std::string parseErrorV;
	const double parsedX = model->parseExpression("LSODEVar[0]", parseSuccessX, parseErrorX);
	const double parsedV = model->parseExpression("LSODEVar[1]", parseSuccessV, parseErrorV);
	if (std::abs(parsedX - 1.0) > 1e-12 || std::abs(parsedV) > 1e-12) {
		std::cerr << "Erro: parser nao leu LSODEVar[0]/LSODEVar[1] com os valores iniciais esperados"
		          << " (LSODEVar[0]=" << parsedX << ", LSODEVar[1]=" << parsedV
		          << ", success0=" << parseSuccessX << ", success1=" << parseSuccessV
		          << ", error0=\"" << parseErrorX << "\", error1=\"" << parseErrorV << "\""
		          << ", raw0=" << state->getValue("0") << ", raw1=" << state->getValue("1") << ")"
		          << std::endl;
		return 1;
	}

	LSODEInitialState initialState{model, time, state};
	model->getOnEventManager()->addOnReplicationStartHandler(&initialState, &LSODEInitialState::OnReplicationStart);

	try {
		model->getSimulation()->start();
	} catch (const std::exception& e) {
		std::cerr << "Erro durante simulacao: " << e.what() << std::endl;
		return 1;
	}

	if (samples.empty()) {
		std::cout << "Teste FALHOU: nenhum ponto foi coletado" << std::endl;
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

	const double xExpected = std::cos(last.t);
	const double vExpected = -std::sin(last.t);
	const double errX = std::abs(last.x - xExpected);
	const double errV = std::abs(last.v - vExpected);
	const double maxEnergyVariationPercent = (maxEnergy - minEnergy) / first.energy * 100.0;

	std::cout << std::fixed << std::setprecision(10);
	std::cout << "t final LSODE = " << last.t << std::endl;
	std::cout << "x final = " << last.x << ", esperado = " << xExpected << ", erro = " << errX << std::endl;
	std::cout << "v final = " << last.v << ", esperado = " << vExpected << ", erro = " << errV << std::endl;
	std::cout << "variacao maxima de energia = " << maxEnergyVariationPercent << "%" << std::endl;
	std::cout << "pontos coletados = " << samples.size() << std::endl;

	if (errX < 1e-4 && errV < 1e-4 && maxEnergyVariationPercent < 1.0) {
		std::cout << "Teste PASSOU: LSODE acompanha cos(t) e -sin(t)" << std::endl;
		return 0;
	}

	std::cout << "Teste FALHOU: LSODE fora da tolerancia esperada" << std::endl;
	return 1;
}
