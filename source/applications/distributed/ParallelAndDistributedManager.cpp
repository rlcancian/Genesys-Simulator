#include <iostream>
#include <fstream>
#include <sstream>
#include <arpa/inet.h>


#include "ParallelAndDistributedManager.h"
#include "Utils.h"

#define EXIT_WITH_ERROR(msg) \
    do { \
        std::cerr << msg << std::endl; \
        exit(EXIT_FAILURE); \
    } while (0)

void ParallelAndDistributedManager::logError(char* msg) {
    this->log(msg, TraceManager::Level::L1_errorFatal);
}

void ParallelAndDistributedManager::logEvent(char* msg) {
    this->log(msg, TraceManager::Level::L5_event);
}

void ParallelAndDistributedManager::log(char* msg, TraceManager::Level level) {
    this->traceHandler(TraceEvent(msg, TraceManager::Level::L5_event));
};

int ParallelAndDistributedManager::main(int argc, char** argv) {
    std::string type;
    std::string modelName;

    if (argc < 2) {
        this->logError("[DPEM] Not enough arguments to start DPEM");
        this->logError("[DPEM] Usage: ./exec <cliente>|<servidor> <nome_modelo> (caso cliente)");
        return 1;
    }

    try {
        type = argv[1];
        if (type == "cliente") {
            this->setIsClient(true);
            modelName = argv[2]; 
        }
    }
    catch(const std::exception& e) {
        this->logError("[DPEM] Failed to convert strings from arg");
    }

    if (this->_simulator->getModels()->loadModel(modelName)) {
        Model* model = this->_simulator->getModels()->front();
        this->_simulator->getModels()->setCurrent(model);
        this->_model = model;

        this->logEvent("[DPEM] Model loaded, starting PDEM execution");

        this->_parallelExecutionManager = new ParallelExecutionManager(model);
        this->_distributedExecutionManager = new DistributedExecutionManager(model);
        
        this->execute(model, modelName);
        return 0;
    }

    this->logError("[DPEM] Failed to load model.");
    return 1;
}

void ParallelAndDistributedManager::execute(Model* model, std::string filename) {
    if (this->getIsClient()) {
        this->traceHandler(TraceEvent("[DPEM] Executing genesys as client", TraceManager::Level::L1_errorFatal));
        
        this->executeClient(filename);
        return;
    };

    this->traceHandler(TraceEvent("[DPEM] Executing genesys as server", TraceManager::Level::L1_errorFatal));
    this->executeServer();
    return;
}

void ParallelAndDistributedManager::executeServer() {
    SocketData* socketDataMainServer = this->_distributedExecutionManager->createNewSocketDataServer(6000);

	_distributedExecutionManager->createServerBind(socketDataMainServer);
    this->logEvent("[DPEM] Creating server bind");
	_distributedExecutionManager->createServerListen(socketDataMainServer);
    this->logEvent("[DPEM] Created server listen");

    this->logEvent("[DPEM] Start listening loop for server");

    //int clientSocketFd = this->_distributedExecutionManager->acceptConnection(socketDataMainServer);
    //SocketData* socketDataAuxServer = this->_distributedExecutionManager->createSocketData(clientSocketFd);
    //this->_distributedExecutionManager->createServerThreadTask(socketDataAuxServer);

    while (true) {
        int clientSocketFd = this->_distributedExecutionManager->acceptConnection(socketDataMainServer);
        
        if (clientSocketFd == -1) {
            std::cerr << "Error accepting connection: " << strerror(errno) << std::endl;
            return;
            // You may want to handle the error in an appropriate way
        } else {
            std::cout << "Connection accepted\n";
        }
        
        this->logEvent("[DPEM] Server connected!");

        SocketData* socketDataAuxServer = this->_distributedExecutionManager->createSocketData(clientSocketFd);

        this->_parallelExecutionManager->createServerThread(
                    &DistributedExecutionManager::createServerThreadTask,
                    _distributedExecutionManager,
                    socketDataAuxServer
                );
    }
}

void ParallelAndDistributedManager::executeClient(std::string filename) {
    this->logEvent("[DPEM] Connecting to servers");
    // TODO Add IP LIST
    // TODO SEMAPHORE FOR READING MODEL
    int socket = this->_distributedExecutionManager->createSocket();
    SocketData* socketItem = this->_distributedExecutionManager->createSocketData(socket);
    socketItem->_address.sin_family = AF_INET;
    socketItem->_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    socketItem->_address.sin_port = htons(6000);

    std::string file = this->modelToFile(filename); 

    this->_distributedExecutionManager->appendSocketDataList(socketItem);

    this->_distributedExecutionManager->connectToServers();

    unsigned int threadNumber = this->_parallelExecutionManager->getThreadNumber();
    //temp
    //this->_distributedExecutionManager->getSocketDataList().at(0)->_replicationNumber = threadNumber;
    //this->_distributedExecutionManager->getSocketDataList().at(0)->_seed = this->_distributedExecutionManager->getRandomSeed();
    //this->_distributedExecutionManager->createClientThreadTask(_distributedExecutionManager->getSocketDataList().at(0), file);

    if (threadNumber >= _distributedExecutionManager->getSocketDataList().size()) {
		threadNumber = _distributedExecutionManager->getSocketDataList().size();
	}

    int replicationsByThread = this->_model->getSimulation()->getNumberOfReplications()/threadNumber;
    
    //int firstSocketId = this->_distributedExecutionManager->getSocketDataList()[0]->_id;

    // TODO - Refactor to stop being a bool
    bool success = this->_parallelExecutionManager->createClientThreads(
        &DistributedExecutionManager::createClientThreadTask,
        _distributedExecutionManager,
        _distributedExecutionManager->getSocketDataList(),
        file,
        replicationsByThread,
        threadNumber
    );

    std::cout << "Ending!!!\n";
}

void ParallelAndDistributedManager::setIsClient(bool boolean) {
    this->_isClient = boolean;
}

bool ParallelAndDistributedManager::getIsClient() { 
    return this->_isClient;
}

std::string ParallelAndDistributedManager::modelToFile(std::string filename) {
    std::fstream file;
	_model->save(filename + ".gen");
	file.open(filename + ".gen", std::ios::in | std::ios::binary);
	std::string contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	return contents;
}

std::string ParallelAndDistributedManager::readFile(const std::string &filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        EXIT_WITH_ERROR("Error opening file");
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    file.close();

    return buffer.str();
}

ParallelAndDistributedManager::ParallelAndDistributedManager()
{
    this->_simulator = new Simulator();
    this->_parallelExecutionManager = nullptr;
    this->_distributedExecutionManager = nullptr;
    this->setIsClient(false);
}

ParallelAndDistributedManager::~ParallelAndDistributedManager()
{
    delete this->_simulator;
    this->_simulator = nullptr;

    this->_parallelExecutionManager = nullptr;
    this->_distributedExecutionManager = nullptr;
}

