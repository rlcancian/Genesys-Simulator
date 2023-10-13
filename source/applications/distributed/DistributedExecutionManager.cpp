#include <thread>
#include "DistributedExecutionManager.h"
#include "../../kernel/simulator/TraceManager.h"
#include "../../kernel/TraitsKernel.h"

DistributedExecutionManager::DistributedExecutionManager() {
    Benchmark::getBenchmarkInfo(&_benchmarkInfo);
}

DistributedExecutionManager::~DistributedExecutionManager() {

}

int DistributedExecutionManager::getNumberThreads() {
    return _benchmarkInfo._nucleos;
}

int DistributedExecutionManager::getRamAmount() {
    return _benchmarkInfo._memoria;
}

unsigned int DistributedExecutionManager::getRandomSeed() {
    srand((unsigned) time(NULL));
    return rand();
}

DistributedExecutionManager::SocketData* DistributedExecutionManager::createNewSocketDataClient(int id) {
    DistributedExecutionManager::SocketData* sdata = new DistributedExecutionManager::SocketData();
    sdata->address.sin_family = AF_INET;
    sdata->address.sin_port = htons(getPort());
    sdata->address.sin_addr.s_addr;
}

bool DistributedExecutionManager::remoteSendExecute(Model* model) {

}

bool DistributedExecutionManager::execute(Model* model) {
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

	for (int i = 0; i < threadNumber; i++) {

		//threadList[i] = std::thread(&getRamAmount);
	};

    model->getSimulation()->start();
}