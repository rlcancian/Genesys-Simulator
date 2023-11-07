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
    DistributedExecutionManager(Model* model);
    ~DistributedExecutionManager();

    // Functions to set and get model passed as parameter to this class
    Model* getModel();
    void setModel(Model* model);

    // Aux functions for benchmark and connectivity
    int getNumberThreads();
    int getRamAmount();
    unsigned int getRandomSeed();

    unsigned int getClientPort();
    unsigned int getServerPort();

    void sendBenchmark();

    // Functions for communication of requests (wrapper of model and number of replications, etc...)
    bool receiveRequestPayload(SocketData* socketData);
    bool sendRequestPayload(SocketData* socketData);

    // Functions for communications of socketData (id, seed, replication number, address and socket)
    bool sendSocketData(SocketData* socketData);
    bool receiveSocketData(SocketData* socketData);

    // Send and receive fileSize of model
    bool receiveFileSize(SocketData* socketData, int& fileSize);
    bool sendFileSize(SocketData* socketData, int fileSize);

    // Send and receive of results (wrapper for all statistics)
    bool receiveResultPayload(SocketData* socketData, ResultPayload& resultPayload);
    bool sendResultPayload(SocketData* socketData);

    // std::vector<std::string> getAvailableIps();
    // void setAvailableIps(std::vector<std::string>);

    std::vector<SocketData*> getSocketDataList();
    void appendSocketDataList(SocketData* socketDataItem);
    
    // create sockets and socketData objects for client and server
    int createSocket();
    SocketData* createClientSocketData(int clientSockfd, const struct sockaddr_in& clientAddress);
    SocketData* createNewSocketDataServer(unsigned int port);
    SocketData* createNewSocketDataClient(unsigned int port, int ipId);
    

    // Used to connect to a server (a instance of genesys listening for connections)
    bool connectToServers();
    bool connectToServerSocket(SocketData* socketData);

    // Receive and send code messages used for communication
    void sendCodeMessage(DistributedCommunication code, int socket);
    bool receiveCodeMessage(DistributedCommunication code, int socket);

    // Server functions
    void createServerBind(SocketData* socketData);
    void createServerListen(SocketData* socketData);

    // Main function executed inside driver code to distribute replications of a model
    bool execute(Model* model);

    // Save current model and output it to stream
    std::string modelToFile();

    // create and close connections with sockets
    void createNewConnection(int socket);
    void closeConnection();

    // main functions for server and client
    void startServerSimulation();
    void startClientSimulation();

    // Thread task functions for client and server threads
    ResultPayload createClientThreadTask(SocketData* socketData);
    ResultPayload createServerThreadTask(SocketData* socketData);

    // Aux functions for debugging
    std::string socketDataToString(SocketData* socketData);
    std::string resultPayloadtoString(ResultPayload* ResultPayload);
    std::string enumToString(DistributedCommunication code) const;
    std::string socketInfoToString(int client_sockfd, const struct sockaddr_in& clientAddress, socklen_t clientLen);

private:
    Benchmark::BenchmarkInfo _benchmarkInfo;
    Model* _model;
    struct pollfd fds[2] = {{.fd = 0, .events = POLLIN}};
    int nfds = 1;
    int max_nfds = 4;
    std::vector<SocketData*> _sockets;
    std::vector<std::string> _ipList;
    int originalNumberOfReplications;
};

#endif