/*
 * Demonstração passo a passo do avanço contínuo acoplado ao relógio de eventos.
 *
 * Objetivo: evidenciar a coerência entre o tempo contínuo do ODESolver e o
 * ModelSimulation::simulatedTime. O programa:
 *   1. imprime na tela uma tabela t, x, v, valores exatos e erro, um ponto por
 *      evento discreto;
 *   2. salva a trajetória completa em "continuous_trajectory.csv" (para plotar);
 *   3. salva o trace bruto em "continuous_trace.log", onde aparecem os eventos
 *      internos "ODESolverStep" intercalados com os eventos discretos, provando
 *      que a integração avança sozinha no calendário do simulador.
 *
 * Sistema: oscilador harmônico  dx/dt = v, dv/dt = -x
 * Solução exata: x(t) = cos(t), v(t) = -sin(t)
 */

#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <iomanip>
#include <sstream>

#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/model/Model.h"
#include "kernel/simulator/PluginManager.h"
#include "plugins/data/Logic/Variable.h"
#include "plugins/data/Continuous/ODESolver.h"
#include "plugins/components/Logic/Create.h"
#include "plugins/components/Continuous/ContinuousSystemComponent.h"
#include "plugins/components/Logic/Dispose.h"
#include "kernel/simulator/essentialPlugins/EntityType.h"
#include "kernel/simulator/TraceManager.h"

struct Sample {
	double t;
	double x;
	double v;
};

// Handler que grava o trace bruto (inclui os passos internos ODESolverStep) em arquivo.
static std::ofstream g_traceFile;
static void writeTraceEventToFile(TraceSimulationEvent e) {
	if (g_traceFile.is_open()) {
		g_traceFile << e.getText() << std::endl;
	}
}

// Componente que registra o estado do solver a cada evento discreto recebido.
class RecordingContinuousSystemComponent : public ContinuousSystemComponent {
public:
	RecordingContinuousSystemComponent(Model* model, ODESolver* solver, std::vector<Sample>* samples, std::string name = "")
		: ContinuousSystemComponent(model, name), _solver(solver), _samples(samples) {
		setOdeSolver(solver);
	}

protected:
	void _onDispatchEvent(Entity* entity, unsigned int inputPortNumber) override {
		ContinuousSystemComponent::_onDispatchEvent(entity, inputPortNumber);
		_samples->push_back({_solver->getCurrentTime(), _solver->getStateValue(0), _solver->getStateValue(1)});
	}

private:
	ODESolver* _solver;
	std::vector<Sample>* _samples;
};

int main() {
	std::cout << "=== Demonstração passo a passo - Oscilador Harmônico ===" << std::endl << std::endl;

	const double twoPi = 2.0 * std::acos(-1.0);
	const unsigned int intervals = 32;                 // 32 eventos discretos ao longo de 2*pi
	const double eventInterval = twoPi / intervals;
	const double replicationLength = twoPi + 1e-9;
	std::ostringstream eventIntervalExpression;
	eventIntervalExpression << std::setprecision(17) << eventInterval;
	std::vector<Sample> samples;

	Simulator* simulator = new Simulator();
	Model* model = simulator->getModelManager()->newModel();

	// Trace no nível interno + handler que grava em arquivo (mostra os ODESolverStep).
	model->getTracer()->setTraceLevel(TraceManager::Level::L7_internal);
	g_traceFile.open("continuous_trace.log");
	model->getTracer()->addTraceSimulationHandler(&writeTraceEventToFile);

	ODESolver* odeSolver = new ODESolver(model, "HarmonicOscillator");
	odeSolver->setTimeVariableName("t");
	odeSolver->setStateVariableNames({"x", "v"});
	odeSolver->setEquationExpressions({"v", "-x"});    // dx/dt = v, dv/dt = -x
	odeSolver->setStep(0.01);
	odeSolver->setStateValues({1.0, 0.0});
	odeSolver->setInitialStateValues({1.0, 0.0});
	// Exportação nativa da trajetória: o próprio ODESolver grava um CSV (t + variáveis de
	// estado) a cada passo. Basta setar este campo — funciona igual em modelos .gen/GUI/CLI.
	odeSolver->setOutputFile("continuous_trajectory.csv");
	model->getDataManager()->insert(odeSolver);

	PluginManager* pluginManager = simulator->getPluginManager();
	if (pluginManager->insert("create.so") == nullptr ||
	    pluginManager->insert("dispose.so") == nullptr ||
	    pluginManager->insert("continuoussystemcomponent.so") == nullptr ||
	    pluginManager->insert("odesolver.so") == nullptr ||
	    pluginManager->insert("variable.so") == nullptr) {
		std::cerr << "Erro ao carregar plugins necessarios" << std::endl;
		return 1;
	}

	EntityType* entityType = pluginManager->newInstance<EntityType>(model, "entitytype");

	Create* create = pluginManager->newInstance<Create>(model, "Create_1");
	create->setTimeBetweenCreationsExpression(eventIntervalExpression.str());
	create->setEntityType(entityType);
	create->setMaxCreations(intervals + 1);

	RecordingContinuousSystemComponent* continuous =
			new RecordingContinuousSystemComponent(model, odeSolver, &samples, "ContinuousSystem_1");
	Dispose* dispose = pluginManager->newInstance<Dispose>(model, "Dispose_1");

	create->connectTo(continuous);
	continuous->connectTo(dispose);

	model->getSimulation()->setReplicationLength(replicationLength);
	model->getSimulation()->setNumberOfReplications(1);

	if (!model->check()) {
		std::cerr << "Erro ao verificar modelo" << std::endl;
		return 1;
	}

	Variable* varT = dynamic_cast<Variable*>(model->getDataManager()->getDataDefinition(Util::TypeOf<Variable>(), "t"));
	Variable* varX = dynamic_cast<Variable*>(model->getDataManager()->getDataDefinition(Util::TypeOf<Variable>(), "x"));
	Variable* varV = dynamic_cast<Variable*>(model->getDataManager()->getDataDefinition(Util::TypeOf<Variable>(), "v"));
	if (varT) varT->setValue(0.0);
	if (varX) varX->setValue(1.0);
	if (varV) varV->setValue(0.0);

	try {
		model->getSimulation()->start();
	} catch (const std::exception& e) {
		std::cerr << "Erro durante simulacao: " << e.what() << std::endl;
		return 1;
	}

	// Tabela passo a passo na tela (um ponto por evento discreto), comparando com a solução exata.
	std::cout << std::fixed << std::setprecision(6);
	std::cout << std::setw(5) << "n" << std::setw(12) << "t"
	          << std::setw(14) << "x" << std::setw(14) << "v"
	          << std::setw(14) << "x_exato" << std::setw(14) << "v_exato"
	          << std::setw(14) << "erro_x" << std::setw(14) << "erro_v" << std::endl;

	for (std::size_t i = 0; i < samples.size(); i++) {
		const Sample& s = samples[i];
		const double xe = std::cos(s.t);
		const double ve = -std::sin(s.t);
		std::cout << std::setw(5) << i << std::setw(12) << s.t
		          << std::setw(14) << s.x << std::setw(14) << s.v
		          << std::setw(14) << xe << std::setw(14) << ve
		          << std::setw(14) << std::abs(s.x - xe) << std::setw(14) << std::abs(s.v - ve) << std::endl;
	}
	g_traceFile.close();

	std::cout << std::endl;
	std::cout << "Pontos na tabela (um por evento discreto): " << samples.size() << std::endl;
	std::cout << "Trajetoria completa (um ponto por passo) salva pelo proprio ODESolver em: continuous_trajectory.csv" << std::endl;
	std::cout << "Trace com os passos internos salvo em: continuous_trace.log" << std::endl;
	std::cout << "  (procure por 'ODESolverStep' para ver a integracao avancando no calendario)" << std::endl;
	return 0;
}
