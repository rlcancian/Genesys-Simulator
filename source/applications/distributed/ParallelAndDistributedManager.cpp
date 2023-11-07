#include <iostream>
#include <fstream>
#include <sstream>

#include "ParallelAndDistributedManager.h"
#include "Utils.h"

void ParallelAndDistributedManager::execute() {
    if (this->isClient) {
        this->executeClient();
        return;
    };

    this->executeServer();
    return;
}

void ParallelAndDistributedManager::executeClient()
{
    this->_distributedExecutionManager->connectToServers();
    int threadNumber = this->_parallelExecutionManager->getThreadNumber();
    
    // TODO - Refactor to stop being a bool
    bool success = this->_parallelExecutionManager->createClientThreads(
        &DistributedExecutionManager::createClientThreadTask,
        _distributedExecutionManager,
        _distributedExecutionManager->getSocketDataList(),
        threadNumber
    );
}

std::string ParallelAndDistributedManager::modelToFile(std::string filename) {
    std::fstream file;
	_model->save(filename + ".gen");
	file.open(filename + ".gen", std::ios::in | std::ios::binary);
	std::string contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	return contents;
}