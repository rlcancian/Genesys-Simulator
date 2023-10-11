#include "../kernel/simulator/Simulator.h"
#include "Benchmark.h"
#include "ParallelExecutionManager.h"
#include "../kernel/util/Util.h"

#ifndef DISTRIBUTED_EXECUTION_MANAGER_H
#define DISTRIBUTED_EXECUTION_MANAGER_H

class DistributedExecutionManager
{
private:
    ParallelExecutionManager parallelExecutionManager;
public:
    Benchmark::BenchmarkInfo benchmarkInfo;

    DistributedExecutionManager();
    ~DistributedExecutionManager();

    int getNumberThreads();
    int getRamAmount();
    
    void setPort();
    int getPort();

    // std::vector<std::string> getAvailableIps();
    // void setAvailableIps(std::vector<std::string>);
    
    // execute only using local threads
    bool localExecute(Model* model);

    // canvas through possible genesys instances
    bool distributedExecute(Model* model);

};

#endif