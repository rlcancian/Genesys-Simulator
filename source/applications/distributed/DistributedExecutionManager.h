#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>

#include "../../kernel/simulator/Simulator.h"
#include "../../kernel/util/Util.h"
#include "Benchmark.h"
#include "Utils.h"

#ifndef DISTRIBUTED_EXECUTION_MANAGER_H
#define DISTRIBUTED_EXECUTION_MANAGER_H

// From a distributed perspective, Client is the instance who will run execute() and will try to find
// another instances of GenesysSimulator running. These instances will be listening (as a server will) on their respective ports
// to execute replications of a model

class DistributedExecutionManager
{
public:
    DistributedExecutionManager(Simulator* simulator);
    ~DistributedExecutionManager();

    // Functions to set and get model passed as parameter to this class
    Model* getModel();
    void setModel(Model* model);
    void setSimulator(Simulator* simulator);

    // Aux functions for benchmark and connectivity
    int getNumberThreads();
    int getRamAmount();
    unsigned int getRandomSeed();

    unsigned int getClientPort();
    unsigned int getServerPort();

    // Functions for communications of socketData (id, seed, replication number, address and socket)
    bool sendSocketData(SocketData* socketData);
    bool receiveSocketData(SocketData* socketData, int socket);

    // Send and receive fileSize of model
    bool receiveFileSize(int socket, int* fileSize);
    bool sendFileSize(int socket, int fileSize);

    // Send and receive of results (wrapper for all statistics)
    bool receiveResultPayload(SocketData* socketData, ResultPayload& resultPayload, std::vector<DataPayload>& dataPayloadList);
    bool sendResultPayload(SocketData* socketData);

    bool sendFileName(int socket, std::string filename);
    bool receiveFileName(int socket, std::string* filename, int fileSize);

    std::vector<SocketData*> getSocketDataList();
    void appendSocketDataList(SocketData* socketDataItem);
    
    // create sockets and socketData objects for client and server
    int createSocket();
    SocketData* createNewSocketDataServer(unsigned int port);
    SocketData* createSocketData(int clientSocketFd);

    // Used to connect to a server (a instance of genesys listening for connections)
    bool connectToServers();
    bool connectToServerSocket(SocketData* socketData);

    // Receive and send code messages used for communication
    bool sendCodeMessage(DistributedCommunication code, int socket);
    bool receiveCodeMessage(DistributedCommunication expectedCode, int socket);

    bool sendModel(std::string file, int socket);
    bool receiveModel(std::string* file, int fileSize, int socket);

    // Server functions
    bool createServerBind(SocketData* socketData);
    void createServerListen(SocketData* socketData);

    void writeToFile(const std::string fileName, const std::string& content);

    void closeConnection(int socketFd);

    // Thread task functions for client and server threads
    ResultPayload createClientThreadTask(SocketData* socketData, std::string filename);
    ResultPayload createServerThreadTask(SocketData* socketData);

    int acceptConnection(SocketData* socketData);

private:
    Benchmark::BenchmarkInfo _benchmarkInfo;
    Model* _model;
    Simulator* _simulator;
    std::vector<SocketData*> _sockets;
    std::vector<std::string> _ipList;
    int originalNumberOfReplications;
    int numberOfReplications;
};

#endif