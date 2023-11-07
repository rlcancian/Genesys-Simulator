#include <thread>
#include <vector>
#include <future>

#include "ParallelExecutionManager.h"
#include "DistributedExecutionManager.h"
#include "Utils.h"

bool ParallelExecutionManager::createClientThreads(
	ResultPayload (DistributedExecutionManager::*func)(SocketData*),
	DistributedExecutionManager* obj,
	std::vector<SocketData*> socketDataList,
	int threadNumber)
{
	if (threadNumber >= socketDataList.size()) {
		threadNumber = socketDataList.size();
	}

	// TODO - How to get number of replications? Add model to attributes of clientThreads? 
	// or add as a parameter

	std::vector<std::future<ResultPayload>> threads;
    std::unordered_map<std::future<ResultPayload>*, std::chrono::time_point<std::chrono::steady_clock>> startTimes;

	for (int i = 0; i < threadNumber; i++) {
		socketDataList[i]->_replicationNumber = repsByThread;
		threads.push_back(std::async(std::launch::async, &DistributedExecutionManager::createClientThreadTask, this, _sockets[i]));
		startTimes[&threads.at(i)] = std::chrono::steady_clock::now();
	}

	// set a timetout
	auto timeout = std::chrono::seconds(20);


	// Use a non-blocking loop to periodically check the status of the futures
    while (!threads.empty()) {
        for (auto it = threads.begin(); it != threads.end();) {
            auto& future = *it;
            auto status = future.wait_for(std::chrono::milliseconds(0));

            if (status == std::future_status::ready) {
                // If the future is ready, retrieve the result
				_model->getTracer()->traceError(TraceManager::Level::L5_event, "[DEM] thread ready!, removing it!");
                ResultPayload result = future.get();
                it = threads.erase(it);

				_model->getTracer()->traceError(TraceManager::Level::L5_event, resultPayloadtoString(&result));
            } else {
                // Check the elapsed time for the future
                auto elapsed = std::chrono::steady_clock::now() - startTimes[&future];

                if (elapsed > timeout) { 
                    // If the future has exceeded the timeout, cancel the operation and erase the future
					_model->getTracer()->traceError(TraceManager::Level::L5_event, "[DEM] TIMEOUT for thread, removing it!");
					// add here a list of sockets to try again (?)
                    it = threads.erase(it);
                } else {
                    ++it;
                }
            }
        }
	}
	//destroy semaphores

	return;
}

bool ParallelExecutionManager::createServerThread(std::future<ResultPayload> (DistributedExecutionManager::*func)(SocketData),
	DistributedExecutionManager* obj,
	SocketData socketData)
{
	
}

int ParallelExecutionManager::getThreadNumber() {
	return std::thread::hardware_concurrency();
}
