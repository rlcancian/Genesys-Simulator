#include <sys/socket.h>
#include <netinet/in.h>
#include "../../kernel/simulator/Simulator.h"
#include "Benchmark.h"
#include "../../kernel/util/Util.h"

#ifndef DISTRIBUTED_EXECUTION_MANAGER_H
#define DISTRIBUTED_EXECUTION_MANAGER_H

// From a distributed perspective, Client is the instance who will run execute() and will try to find
// another instances of GenesysSimulator running. These instances will be listening (as a server will) on their respective ports
// to execute replications of a model

class DistributedExecutionManager
{
private:
    Benchmark::BenchmarkInfo _benchmarkInfo;
public:
    struct SocketData {
        int id;
        unsigned int seed;
        unsigned int replicationNumber;
        struct sockaddr_in address;
    };

    struct ModelExecutionPayload {
        SocketData socketData;
        Model model;
    };

    DistributedExecutionManager();
    ~DistributedExecutionManager();

    int getNumberThreads();
    int getRamAmount();
    unsigned int getRandomSeed();

    void setPort();
    int getPort();

    // std::vector<std::string> getAvailableIps();
    // void setAvailableIps(std::vector<std::string>);
    

    SocketData* createNewSocketDataClient(int id);
    SocketData* createNewSocketDataServer(int id);
    

    // Used to connect to a server (a instance of genesys listening for connections)
    bool connectToServer();

    // Main function executed inside driver code to distribute replications of a model
    bool execute(Model* model);

    // Function thread uses to pass necessary info to execute distributed replications of a model
    bool remoteSendExecute(Model* model);

    

    // // canvas through possible genesys instances
    // bool distributedExecute(Model* model);

};

#endif