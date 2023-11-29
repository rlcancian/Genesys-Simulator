#include <future>

#include "Utils.h"
#include "DistributedExecutionManager.h"


#ifndef PARALLEL_EXECUTION_MANAGER_H
#define PARALLEL_EXECUTION_MANAGER_H

//TODO
// Create semaphore executing model
// create semaphore for using file (?) only needed when client and server are being executed from the same genesys

class ParallelExecutionManager
{
private:
    Model* _model;
public:
    ParallelExecutionManager(Model* model) { this->_model = model; }
    ~ParallelExecutionManager() { this->_model = nullptr; }
    unsigned int getThreadNumber();

    // Based on how many possible genesys instances there are, create a number of
    // client threads to handle them
    bool createClientThreads(
        // DistributedExecutionManager function that is being passed to each client thread
        ResultPayload (DistributedExecutionManager::*func)(SocketData*, std::string),
        DistributedExecutionManager* obj,
        std::vector<SocketData*> socketDataList,
        std::string model,
        int replicationsByThread,
        int threadNumber
    );

    // Used to create a single future thread each time. Returns a future (promise)
    ResultPayload createServerThread(ResultPayload (DistributedExecutionManager::*func)(SocketData*),
        DistributedExecutionManager* obj,
        SocketData* socketData
    );
};

#endif