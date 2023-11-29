#include <iostream>
#include <fstream>
#include <sstream>
#include <arpa/inet.h>


#include "ParallelAndDistributedManager.h"
#include "Utils.h"
#include "../terminal/examples/arenaExamples/AirportSecurityExampleExtended.h"

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

    this->_simulator = new Simulator();
    this->_model = new Model(this->_simulator);

    // if (this->getIsClient()) {
    //     std::string modelToString;
    //     if (!this->readFile(modelName, &modelToString)) {
    //         this->logError("[DPEM] Error trying to open file");
    //         return 1;
    //     }

    //     this->logEvent("File opened and model is this: ----------\n");
    //     this->logEvent(const_cast<char*>(modelToString.c_str()));
    //     this->logEvent("End of model ---------\n");

    //     this->insertFakePluginsByHand(this->_simulator);
        
    //     std::string path = "../../models/";

    //     if (!this->_model->load(path.append(modelName))) {
    //         this->logError("[DPEM] Failed to load model.");
    //         return 1;
    //     }

    //     this->_simulator->getModels()->setCurrent(this->_model);
    //     this->logEvent("[DPEM] Model loaded, starting PDEM execution");
    // }

    this->setDefaultTraceHandlers(this->_simulator->getTracer());

    if (this->getIsClient()) {
        this->insertFakePluginsByHand(this->_simulator);
        this->createModelTemp();
    }
    
    logEvent("Executing PDEM Right Now\n");

    this->_distributedExecutionManager = new DistributedExecutionManager(this->_model);
    this->_parallelExecutionManager = new ParallelExecutionManager(this->_model);
    this->execute(modelName);

    return 0;
}

void ParallelAndDistributedManager::execute(std::string filename) {
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

    if (threadNumber >= _distributedExecutionManager->getSocketDataList().size()) {
		threadNumber = _distributedExecutionManager->getSocketDataList().size();
	}

    int replicationsByThread = this->_model->getSimulation()->getNumberOfReplications()/threadNumber;
    

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
	_model->save(filename);
	file.open(filename, std::ios::in | std::ios::binary);
	std::string contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	return contents;
}

bool ParallelAndDistributedManager::readFile(const std::string &filename, std::string* fileOutput) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        // Return false if the file cannot be opened
        this->logError("Error opening file\n");
        return false;
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    file.close();

    // Store the content in the pointed std::string
    *fileOutput = buffer.str();

    // Return true indicating success
    return true;
}

void ParallelAndDistributedManager::createModelTemp() {
    PluginManager* plugins = this->_simulator->getPlugins();
    
    plugins->newInstance<Attribute>(this->_model, "a_Time_in");
    plugins->newInstance<Variable>(this->_model, "v_Counter");

    Create* create = plugins->newInstance<Create>(this->_model);
    create->setDescription("Passengers arrive to security");
    create->setEntityTypeName("Passenger");
    create->setTimeBetweenCreationsExpression("expo(2)");
    create->setTimeUnit(Util::TimeUnit::minute);
    create->setEntitiesPerCreation(1);
    create->setFirstCreation(0.0);

    Resource* officer = plugins->newInstance<Resource>(this->_model, "Officer");
    officer->setCapacity(1);
    officer->setCostBusyTimeUnit(12);
    officer->setCostIdleTimeUnit(12);


    Resource* xray = plugins->newInstance<Resource>(this->_model, "XRay Machine");
    xray->setCapacity(2);

    Assign* assignTimeIn = plugins->newInstance<Assign>(this->_model);
    assignTimeIn->getAssignments()->insert(new Assignment(this->_model, "a_Time_in", "tnow", true));

    Process* processIdentification = plugins->newInstance<Process>(this->_model);
    processIdentification->setDescription("Check for proper identification");
    processIdentification->getSeizeRequests()->insert(new SeizableItem(officer, "1"));
    processIdentification->setQueueableItem(new QueueableItem(this->_model, "Identification.Queue"));
    processIdentification->setDelayExpression("tria(0.75, 1.5, 3)");
    processIdentification->setDelayTimeUnit(Util::TimeUnit::minute);

    Decide* decidePassSecurity = plugins->newInstance<Decide>(this->_model);
    decidePassSecurity->setDescription("Pass security?");
    decidePassSecurity->getConditions()->insert("unif(0, 1) < 0.96");

    Dispose* disposeCleared = plugins->newInstance<Dispose>(this->_model);
    disposeCleared->setDescription("Cleared");

    Dispose* disposeDenied = plugins->newInstance<Dispose>(this->_model);
    disposeDenied->setDescription("Denied");

    Process* processXray = plugins->newInstance<Process>(this->_model);
    processXray->setDescription("XRay Baggage Check");
    processXray->getSeizeRequests()->insert(new SeizableItem(xray, "1"));
    processXray->setQueueableItem(new QueueableItem(this->_model, "XRay.Queue"));
    processXray->setDelayExpression("tria(1.75, 2.5, 7)");
    processXray->setDelayTimeUnit(Util::TimeUnit::minute);

    Assign* assignCounter = plugins->newInstance<Assign>(this->_model);
    assignCounter->getAssignments()->insert(new Assignment(this->_model, "v_Counter", "v_Counter + 1", false));

    Decide* decideExtraScreening = plugins->newInstance<Decide>(this->_model);
    decideExtraScreening->setDescription("Extra screening for 15th?");
    decideExtraScreening->getConditions()->insert("v_Counter == 15");

    Assign* resetCounter = plugins->newInstance<Assign>(this->_model);
    resetCounter->getAssignments()->insert(new Assignment(this->_model, "v_Counter", "0", false));

    Process* extraSecurityCheck = plugins->newInstance<Process>(this->_model);
    extraSecurityCheck->setDescription("Additional Security Check");
    extraSecurityCheck->setQueueableItem(new QueueableItem(this->_model, "Additional.Queue"));
    extraSecurityCheck->setDelayExpression("tria(3, 5, 10)");
    extraSecurityCheck->setDelayTimeUnit(Util::TimeUnit::minute);

    Record* cycleTimeRecord = plugins->newInstance<Record>(this->_model);
    cycleTimeRecord->setExpressionName("Cycle Time for Selected Passengers");
    cycleTimeRecord->setExpression("a_Time_in");

    Dispose* disposeClearedWithExtraCheck = plugins->newInstance<Dispose>(this->_model);
    disposeClearedWithExtraCheck->setDescription("Cleared with extra check");

    create->getConnections()->insert(assignTimeIn);
    assignTimeIn->getConnections()->insert(processIdentification);
    processIdentification->getConnections()->insert(decidePassSecurity);
    decidePassSecurity->getConnections()->insert(processXray);
        processXray->getConnections()->insert(assignCounter);
        assignCounter->getConnections()->insert(decideExtraScreening);
        decideExtraScreening->getConnections()->insert(resetCounter);
            resetCounter->getConnections()->insert(extraSecurityCheck);
            extraSecurityCheck->getConnections()->insert(cycleTimeRecord);
            cycleTimeRecord->getConnections()->insert(disposeClearedWithExtraCheck);
        decideExtraScreening->getConnections()->insert(disposeCleared);
    decidePassSecurity->getConnections()->insert(disposeDenied);

    //genesys->getTracer()->setTraceLevel(TraceManager::Level::L9_mostDetailed);
    this->_simulator->getTracer()->setTraceLevel(TraceManager::Level::L2_results);
    this->_model->getSimulation()->setReplicationLength(2, Util::TimeUnit::day);
    this->_model->getSimulation()->setNumberOfReplications(300);
    this->_model->getSimulation()->setWarmUpPeriod(0.1);
    this->_model->getSimulation()->setWarmUpPeriodTimeUnit(Util::TimeUnit::day);
    
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
    delete this->_model;
    this->_simulator = nullptr;
    this->_parallelExecutionManager = nullptr;
    this->_distributedExecutionManager = nullptr;
}