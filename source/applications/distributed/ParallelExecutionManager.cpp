#include <thread>
#include <vector>
#include <future>

#include "ParallelExecutionManager.h"
#include "DistributedExecutionManager.h"
#include "Utils.h"

bool ParallelExecutionManager::createClientThreads(
    ResultPayload (DistributedExecutionManager::*func)(SocketData*, std::string),
	DistributedExecutionManager* obj,
	std::vector<SocketData*> socketDataList,
	std::string file,
	int replicationsByThread,
	int threadNumber)
{
	std::vector<std::future<ResultPayload>> threads;
    std::unordered_map<std::future<ResultPayload>*, std::chrono::time_point<std::chrono::steady_clock>> startTimes;


	for (int i = 0; i < threadNumber; i++) {
        std::cout << "[DEM] Launching future\n";
        srand((unsigned) time(NULL));

		socketDataList[i]->_replicationNumber = replicationsByThread;
        socketDataList[i]->_id = i;
        socketDataList[i]->_seed = rand();

		threads.push_back(std::async(std::launch::async, func, obj, socketDataList[i], file));
		startTimes[&threads.at(i)] = std::chrono::steady_clock::now();
	}

	auto timeout = std::chrono::seconds(10);

	return true;
}

ResultPayload ParallelExecutionManager::createServerThread(ResultPayload (DistributedExecutionManager::*func)(SocketData*),
	DistributedExecutionManager* obj,
	SocketData* socketData
    )
{
    ResultPayload result;
    std::cout << "launching future for server\n";
    std::future<ResultPayload> resultPayloadFuture = std::async(std::launch::async, func, obj, socketData);

    auto startTime = std::chrono::steady_clock::now();

    while (true) {
        auto futureState = resultPayloadFuture.wait_for(std::chrono::seconds(0));

        auto currentTime = std::chrono::steady_clock::now();
        if (currentTime - startTime > std::chrono::seconds(20)) {
            std::cout << "Server thread TIMED OUT\n";
            result.code = DistributedCommunication::FAILURE;
            result.threadId = -1;
            return result;
        }

        if (futureState == std::future_status::ready) {
            break;
        }
    }

    resultPayloadFuture.get();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::cout << "[DEM] AUX Server thread returned normally\n";
    return result;
}
unsigned int ParallelExecutionManager::getThreadNumber()
{
    return std::thread::hardware_concurrency();
}
