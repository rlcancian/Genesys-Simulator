#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>

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
public:
    struct SocketData {
        int _id;
        unsigned int _seed;
        unsigned int _replicationNumber;
        struct sockaddr_in _address;
        int _socket;
    };

    struct ModelExecutionPayload {
        SocketData socketData;
    };

    enum DistributedCommunication {
        INIT_CONNECTION = 1,
        SEND_MODEL = 2,
        RECEIVE_DATA = 3,
        RESULTS = 4,
        CLOSE_CONNECTION = 5,
        NOTHING = 6,
        FAILURE = 7,
        BENCHMARK = 8
    };

public:
    DistributedExecutionManager(Model* model);
    ~DistributedExecutionManager();

    Model* getModel();
    void setModel(Model* model);

    int getNumberThreads();
    int getRamAmount();
    unsigned int getRandomSeed();

    void setPort();
    unsigned int getClientPort();
    unsigned int getServerPort();

    void sendBenchmark();

    bool receivePayload(SocketData* socketData);
    bool sendPayload(SocketData* socketData);

    void sendSocketData(SocketData* socketData);
    bool receiveSocketData(SocketData* socketData);

    uint64_t receiveFileSize(SocketData* socketData);

    // std::vector<std::string> getAvailableIps();
    // void setAvailableIps(std::vector<std::string>);

    std::vector<SocketData*>* getSocketDataList();
    void appendSocketDataList(SocketData* socketDataItem);
    
    int createSocket();
    SocketData* createNewSocketDataClient(unsigned int port, int ipId);
    SocketData* createNewSocketDataServer(unsigned int port);
    

    // Used to connect to a server (a instance of genesys listening for connections)
    bool connectToServers();
    bool connectToServerSocket(SocketData* socketData);


    bool connectToClient(SocketData* socketData);

    void sendCodeMessage(DistributedCommunication code, int socket);
    bool receiveCodeMessage(DistributedCommunication code, int socket);

    void createServerBind(SocketData* socketData);
    void createServerListen(SocketData* socketData);

    // Main function executed inside driver code to distribute replications of a model
    bool execute(Model* model);

    // Function thread uses to pass necessary info to execute distributed replications of a model
    bool remoteSendExecute(Model* model);

    std::string modelToFile();

    // // canvas through possible genesys instances
    // bool distributedExecute(Model* model);

    DistributedCommunication getNextDistributedCommunicationCode();

    void createNewConnection(int socket);
    void closeConnection();

    void startServerSimulation();
    void startClientSimulation();

    void createClientThreadTask(SocketData* socketData);

private:
    Benchmark::BenchmarkInfo _benchmarkInfo;
    Model* _model;
    struct pollfd fds[2] = {{.fd = 0, .events = POLLIN}};
    int nfds = 1;
    int max_nfds = 4;
    std::vector<SocketData*> _sockets;
    std::vector<std::string> _ipList;
};

#endif