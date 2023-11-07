#include <future>

#include "Utils.h"
#include "DistributedExecutionManager.h"


#ifndef PARALLEL_EXECUTION_MANAGER_H
#define PARALLEL_EXECUTION_MANAGER_H

class ParallelExecutionManager
{
private:
    /* data */
public:
    ParallelExecutionManager(/* args */);
    ~ParallelExecutionManager();
    int getThreadNumber();

    // Based on how many possible genesys instances there are, create a number of
    // client threads to handle them
    bool createClientThreads(
        // DistributedExecutionManager function that is being passed to each client thread
        ResultPayload (DistributedExecutionManager::*func)(SocketData*),
        DistributedExecutionManager* obj,
        std::vector<SocketData*> socketDataList,
        int threadNumber
    );

    // Used to create a single future thread each time. Returns a future (promise)
    bool createServerThread(
        std::future<ResultPayload> (DistributedExecutionManager::*func)(SocketData),
        DistributedExecutionManager* obj,
        SocketData socketData
    );

};

ParallelExecutionManager::ParallelExecutionManager(/* args */)
{
}

ParallelExecutionManager::~ParallelExecutionManager()
{
}

#endif