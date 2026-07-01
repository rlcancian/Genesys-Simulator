/*
 * Teste para ContinuousSystemComponent com oscilador harmônico
 * 
 * Sistema: dx/dt = v, dv/dt = -x
 * Solução exata: x(t) = cos(t), v(t) = -sin(t)
 * 
 * Modelo:
 * - Create (inter-arrival time = 0.1)
 * - ContinuousSystemComponent (referenciando ODESolver do oscilador harmônico)
 * - Dispose
 */

#include <iostream>
#include <cmath>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <sstream>

#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/Model.h"
#include "kernel/simulator/PluginManager.h"
#include "kernel/simulator/Plugin.h"
#include "plugins/data/Logic/Variable.h"
#include "plugins/data/Continuous/ODESolver.h"
#include "plugins/components/Logic/Create.h"
#include "plugins/components/Continuous/ContinuousSystemComponent.h"
#include "plugins/components/Logic/Dispose.h"
#include "kernel/simulator/EntityType.h"
#include "kernel/simulator/TraceManager.h"

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

int main() {
	std::cout << "=== Teste ContinuousSystemComponent - Oscilador Harmônico ===" << std::endl;
	std::cout << std::endl;

	const double twoPi = 2.0 * std::acos(-1.0);
	const unsigned int intervals = 64;
	const double eventInterval = twoPi / intervals;
	const double replicationLength = twoPi + 1e-9;
	std::ostringstream eventIntervalExpression;
	eventIntervalExpression << std::setprecision(17) << eventInterval;
	std::vector<Sample> samples;

	// Criar simulador e modelo
	Simulator* simulator = new Simulator();
	Model* model = simulator->getModelManager()->newModel();

	// Configurar trace level para ver os logs
	model->getTracer()->setTraceLevel(TraceManager::Level::L3_errorRecover);

	// Criar ODESolver para o oscilador harmônico
	ODESolver* odeSolver = new ODESolver(model, "HarmonicOscillator");
	odeSolver->setTimeVariableName("t");

	std::vector<std::string> stateVars = {"x", "v"};
	odeSolver->setStateVariableNames(stateVars);

	std::vector<std::string> equations = {"v", "-x"};  // dx/dt = v, dv/dt = -x
	odeSolver->setEquationExpressions(equations);

	odeSolver->setStep(0.01);
	odeSolver->setPrecision(1e-6);
	odeSolver->setMaxSteps(100000);

	// Definir valores iniciais
	std::vector<double> initialValues = {1.0, 0.0};
	odeSolver->setStateValues(initialValues);
	odeSolver->setInitialStateValues(initialValues);

	model->getDataManager()->insert(odeSolver);

	// Registrar plugins estáticos manualmente no PluginManager
	// Isso é necessário porque o model check precisa encontrar os plugins
	PluginManager* pluginManager = simulator->getPluginManager();
	
	// Criar e inserir os plugins necessários
	pluginManager->insertStaticPlugin(new Plugin(&Create::GetPluginInformation));
	pluginManager->insertStaticPlugin(new Plugin(&Dispose::GetPluginInformation));
	pluginManager->insertStaticPlugin(new Plugin(&ContinuousSystemComponent::GetPluginInformation));
	pluginManager->insertStaticPlugin(new Plugin(&ODESolver::GetPluginInformation));
	pluginManager->insertStaticPlugin(new Plugin(&Variable::GetPluginInformation));
	pluginManager->insertStaticPlugin(new Plugin(&EntityType::GetPluginInformation));

	// Criar EntityType
	EntityType* entityType = pluginManager->newInstance<EntityType>(model, "entitytype");

	// Criar componentes usando o PluginManager
	Create* create = pluginManager->newInstance<Create>(model, "Create_1");
	create->setTimeBetweenCreationsExpression(eventIntervalExpression.str());
	create->setEntityType(entityType);
	create->setMaxCreations(intervals + 1);

	RecordingContinuousSystemComponent* continuous =
			new RecordingContinuousSystemComponent(model, odeSolver, &samples, "ContinuousSystem_1");

	Dispose* dispose = pluginManager->newInstance<Dispose>(model, "Dispose_1");

	// Conectar componentes: Create -> ContinuousSystem -> Dispose
	create->connectTo(continuous);
	continuous->connectTo(dispose);

	// Configurar simulação para rodar até t=2*pi
	model->getSimulation()->setReplicationLength(replicationLength);
	model->getSimulation()->setNumberOfReplications(1);

	// Verificar o modelo
	if (!model->check()) {
		std::cerr << "Erro ao verificar modelo" << std::endl;
		return 1;
	}
	std::cout << "Modelo verificado com sucesso!" << std::endl;

	// Set initial values for variables (ODESolver creates them during check)
	Variable* varT = dynamic_cast<Variable*>(model->getDataManager()->getDataDefinition(Util::TypeOf<Variable>(), "t"));
	Variable* varX = dynamic_cast<Variable*>(model->getDataManager()->getDataDefinition(Util::TypeOf<Variable>(), "x"));
	Variable* varV = dynamic_cast<Variable*>(model->getDataManager()->getDataDefinition(Util::TypeOf<Variable>(), "v"));

	if (varT) varT->setValue(0.0);
	if (varX) varX->setValue(1.0);  // Condição inicial: x = 1.0
	if (varV) varV->setValue(0.0);  // Condição inicial: v = 0.0

	std::cout << "Modelo configurado com sucesso!" << std::endl;
	std::cout << "Condições iniciais: t=0.0, x=1.0, v=0.0" << std::endl;
	std::cout << std::setprecision(12);
	std::cout << "Inter-arrival time: " << eventInterval << std::endl;
	std::cout << "Replication target t=2*pi: " << twoPi << std::endl;
	std::cout << "Replication length configured: " << replicationLength << std::endl;
	std::cout << std::endl;
	std::cout << "=== Iniciando simulação ===" << std::endl;
	std::cout << std::endl;

	// Executar simulação
	try {
		model->getSimulation()->start();
	} catch (const std::exception& e) {
		std::cerr << "Erro durante simulação: " << e.what() << std::endl;
		return 1;
	}

	std::cout << std::endl;
	std::cout << "=== Simulação concluída ===" << std::endl;
	std::cout << std::endl;

	if (samples.empty()) {
		std::cout << "✗ Teste FALHOU: nenhum ponto foi coletado" << std::endl;
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

	const double finalEnergyVariationPercent =
			std::abs(last.energy - first.energy) / first.energy * 100.0;
	const double maxEnergyVariationPercent =
			(maxEnergy - minEnergy) / first.energy * 100.0;

	// Mostrar valores finais
	std::cout << "Valores finais:" << std::endl;
	std::cout << "  t real final = " << last.t << std::endl;
	std::cout << "  x_final = " << last.x << std::endl;
	std::cout << "  v_final = " << last.v << std::endl;
	std::cout << "x via odeSolver = " << odeSolver->getStateValue(0) << std::endl;
	std::cout << "v via odeSolver = " << odeSolver->getStateValue(1) << std::endl;
	std::cout << "Energia:" << std::endl;
	std::cout << "  E inicial = " << first.energy << std::endl;
	std::cout << "  E final = " << last.energy << std::endl;
	std::cout << "  variação percentual final = " << finalEnergyVariationPercent << "%" << std::endl;
	std::cout << "  variação percentual máxima ao longo da simulação = "
	          << maxEnergyVariationPercent << "%" << std::endl;
	std::cout << "  pontos coletados = " << samples.size() << std::endl;
	std::cout << std::endl;

	// Valores esperados em t=2*pi
	double x_expected = std::cos(twoPi);
	double v_expected = -std::sin(twoPi);

	std::cout << "Valores esperados (solução exata em t=2*pi):" << std::endl;
	std::cout << "  x = " << x_expected << std::endl;
	std::cout << "  v = " << v_expected << std::endl;
	std::cout << std::endl;

	// Calcular erros
	double err_x = std::abs(last.x - x_expected);
	double err_v = std::abs(last.v - v_expected);

	std::cout << "Erros:" << std::endl;
	std::cout << "  err_x = " << err_x << std::endl;
	std::cout << "  err_v = " << err_v << std::endl;
	std::cout << std::endl;

	// Verificar se os erros são aceitáveis
	if (err_x < 1e-4 && err_v < 1e-4 && maxEnergyVariationPercent < 1.0) {
		std::cout << "✓ Teste PASSOU: integração contínua numericamente estável em t=2*pi" << std::endl;
		std::cout << "✓ x_final próximo de 1.0, v_final próximo de 0.0" << std::endl;
		std::cout << "✓ Energia variou menos de 1% ao longo da simulação" << std::endl;
		return 0;
	} else {
		std::cout << "✗ Teste FALHOU: estabilidade numérica fora dos limites esperados" << std::endl;
		return 1;
	}
}
