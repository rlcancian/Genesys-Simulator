#include <thread>
#include "DistributedExecutionManager.h"
#include "../../kernel/simulator/TraceManager.h"
#include "../../kernel/TraitsKernel.h"

DistributedExecutionManager::DistributedExecutionManager() {
    Benchmark::getBenchmarkInfo(&benchmarkInfo);
}

DistributedExecutionManager::~DistributedExecutionManager() {

}

int DistributedExecutionManager::getNumberThreads() {
    return 2;
}

int DistributedExecutionManager::getRamAmount() {
    return benchmarkInfo._memoria;
}

bool DistributedExecutionManager::localExecute(Model* model) {
    // Criar varios modelos que possuam um numero de 
    // replicacoes para executar
    int replications = model->getSimulation()->getNumberOfReplications();

    if (replications == 1) {
        model->getTracer()->traceError(TraceManager::Level::L4_warning, "Only one replication, not using distributed and parallel execution");
        model->getSimulation()->start();
        return true;
    };

    int threadNumber = getNumberThreads();
    int replicationsByThread = replications/threadNumber;

    std::thread threadList[threadNumber];
	std::thread threadStart;
	//thread_start = std::thread(&modelSimSimulation::start, this);

	for (int i = 0; i < threadNumber; i++) {
		threadList[i] = std::thread(&getRamAmount);
	}

    //modelSim->getSimulation()->start();
}