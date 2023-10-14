#include <thread>
#include <netinet/in.h>
#include <poll.h>
#include <arpa/inet.h>

#include "DistributedExecutionManager.h"
#include "../../kernel/simulator/TraceManager.h"
#include "../../kernel/TraitsKernel.h"

DistributedExecutionManager::DistributedExecutionManager(Model* model) {
    setModel(model);
    Benchmark::getBenchmarkInfo(&_benchmarkInfo);
}

DistributedExecutionManager::~DistributedExecutionManager() {}

void DistributedExecutionManager::setModel(Model* model) { _model = model; }

Model* DistributedExecutionManager::getModel() { return _model; }

unsigned int DistributedExecutionManager::getClientPort() { return 6666; }

unsigned int DistributedExecutionManager::getServerPort() { return 6000; }

int DistributedExecutionManager::getNumberThreads() { return _benchmarkInfo._nucleos; }

int DistributedExecutionManager::getRamAmount() { return _benchmarkInfo._memoria; }

std::vector<DistributedExecutionManager::SocketData*>* DistributedExecutionManager::getSocketDataList() { return &_sockets; }

void DistributedExecutionManager::appendSocketDataList(SocketData* socketDataItem) {  _sockets.insert(_sockets.end(), socketDataItem); }

unsigned int DistributedExecutionManager::getRandomSeed() {
    srand((unsigned) time(NULL));
    return rand();
}

int DistributedExecutionManager::createSocket() {
	int _socket = socket(AF_INET, SOCK_STREAM, 0);
	if (_socket == 0) {
		_model->getTracer()->traceError(TraceManager::Level::L1_errorFatal, "[ERROR - FATAL] Fail to create a socket.");
		exit(-1);
	}
	return _socket;
}

void DistributedExecutionManager::createServerBind(DistributedExecutionManager::SocketData* socketData) {
	bind(socketData->_socket, (struct sockaddr *) &socketData->_address, sizeof(socketData->_address));
}

void DistributedExecutionManager::createServerListen(DistributedExecutionManager::SocketData* socketData) {
	int max_nfds = 2;
    listen(socketData->_socket, max_nfds);
}

bool DistributedExecutionManager::connectToServers() {
    // Temporary
    //Create IPList and iterate through 
    // for i in range(ipList.size)...
    //    tempSocketData = createNewSocketDataCliente(i)
    DistributedExecutionManager::SocketData* tempSocketData = createNewSocketDataClient(0, 0);
    if (connectToServerSocket(tempSocketData)) {
        sendCodeMessage(DistributedCommunication::INIT_CONNECTION, tempSocketData->_socket);
        if (receiveCodeMessage(DistributedCommunication::INIT_CONNECTION, tempSocketData->_socket)) {
            appendSocketDataList(tempSocketData);
        } else {
            _model->getTracer()->traceError(TraceManager::Level::L1_errorFatal, "[ERROR - FATAL] Instance didn't return the expected command");
        }
    } else {
        _model->getTracer()->traceError(TraceManager::Level::L1_errorFatal, "[ERROR - FATAL] Connection couldn't be established with instance");
    }
}



bool DistributedExecutionManager::receiveCodeMessage(DistributedExecutionManager::DistributedCommunication code, int socket = -1) {
	int client_socket = (socket == -1) ? fds[1].fd : socket;
	int buffer = -1;
	int p = recv(client_socket, &buffer, sizeof(int), 0);
	if (p > 0) {
		DistributedExecutionManager::DistributedCommunication _code;
		_code = (DistributedExecutionManager::DistributedCommunication)ntohl(buffer);
		std::string msg_code = "[EVENT] Received code " + std::to_string((int)_code);
		_model->getTracer()->traceError(TraceManager::Level::L5_event, msg_code);
		return (_code == code);
	} else {
		return false;
	}
	return false;
}

DistributedExecutionManager::SocketData* DistributedExecutionManager::createNewSocketDataClient(unsigned int port, int ipId) {
    DistributedExecutionManager::SocketData* socketData = new DistributedExecutionManager::SocketData();
	socketData->_id = -1;
	socketData->_seed = -1;
	socketData->_replicationNumber = -1;
	socketData->_socket = createSocket();
	socketData->_address.sin_family = AF_INET;
	//socketData->_address.sin_addr.s_addr = inet_addr(_ipList.at(ipId).c_str());
    socketData->_address.sin_addr.s_addr = inet_addr("6666");//
    
	socketData->_address.sin_port = htons(port);
    return socketData;
}

DistributedExecutionManager::SocketData* DistributedExecutionManager::createNewSocketDataServer(unsigned int port) {
	SocketData* socketData = new DistributedExecutionManager::SocketData();
	socketData->_id = -1;
	socketData->_seed = -1;
	socketData->_replicationNumber = -1;
	socketData->_socket = createSocket();
	socketData->_address.sin_family = AF_INET;
	socketData->_address.sin_addr.s_addr = htonl(INADDR_ANY);
	socketData->_address.sin_port = htons(port);
	int on = 1;
	setsockopt(socketData->_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&on,sizeof(on));
	fds[0].fd = socketData->_socket;
	return socketData;
}


bool DistributedExecutionManager::connectToServerSocket(DistributedExecutionManager::SocketData* socketData) {
	if ((connect(socketData->_socket, (struct sockaddr *) &socketData->_address, sizeof(socketData->_address))) < 0) {
		getModel()->getTracer()->traceError(TraceManager::Level::L1_errorFatal, "[ERROR - FATAL] Failed to connect to the server.");
		return false;
	}
	getModel()->getTracer()->traceError(TraceManager::Level::L5_event, "[EVENT] Connected to the server.");
	return true;
}

void DistributedExecutionManager::sendCodeMessage(DistributedExecutionManager::DistributedCommunication code, int socket = -1) {
	int client_socket = (socket == -1) ? fds[1].fd : socket;
	int msg = htonl((int)code);
	send(client_socket, &msg, sizeof(int), 0);
	std::string msg_code = "Sending code " + std::to_string((int)code) + "...";
	_model->getTracer()->traceError(TraceManager::Level::L5_event, msg_code);
}

bool DistributedExecutionManager::execute(Model* model) {
    // Criar varios modelos que possuam um numero de 
    // replicacoes para executar
    int replications = model->getSimulation()->getNumberOfReplications();

    if (replications == 1) {
        model->getTracer()->traceError(TraceManager::Level::L4_warning, "Only one replication, not using distributed and parallel execution");
        model->getSimulation()->start();
        return true;
    };

    int threadNumber = getNumberThreads();
    int replicationsByThread = replications/threadNumber;

    std::thread threadList[threadNumber];

	for (int i = 0; i < threadNumber; i++) {

		//threadList[i] = std::thread(&getRamAmount);
	};

    model->getSimulation()->start();
}

DistributedExecutionManager::DistributedCommunication DistributedExecutionManager::getNextDistributedCommunicationCode() {
	poll(fds, nfds, -1);
	int buffer = -1;

	for (int i = 0; i < nfds; i++) {
		switch (fds[i].revents) {
			case 0:
				break;
			case POLLIN:
			{
				if (fds[i].fd == fds[0].fd) {
					struct sockaddr_in addr;
					int socklen, newsock, bytes, ret;
					char op, str[INET_ADDRSTRLEN];
					newsock = accept(fds[0].fd, NULL, NULL);
					if (nfds == max_nfds) {
						_model->getTracer()->traceError(TraceManager::Level::L4_warning, "Network error. Client limit reached.");
						close(newsock);
						return DistributedExecutionManager::DistributedCommunication::FAILURE;
					} else {
						_model->getTracer()->traceError(TraceManager::Level::L5_event, "Network connection. New client connected.");
						createNewConnection(newsock);
					}
					fflush(stdout);
					return DistributedExecutionManager::DistributedCommunication::NOTHING;
				}
				int p = recv(fds[i].fd, &buffer, sizeof(int), 0);
				if (p > 0) {
					DistributedExecutionManager::DistributedCommunication code;
					code = (DistributedExecutionManager::DistributedCommunication)ntohl(buffer);
					_model->getTracer()->traceError(TraceManager::Level::L5_event, "Network event. Client send a code.");
					std::cout << (int)code << std::endl;
					fflush(stdout);
					return code;
				} else {
					endConnection();
					DistributedExecutionManager::DistributedCommunication::NOTHING;
				}
				break;
			}
			case POLLNVAL:
			case POLLPRI:
			case POLLOUT:
			case POLLERR:
			case POLLHUP:
			default:
				_model->getTracer()->traceError(TraceManager::Level::L4_warning, "Network error. Unespected revents.");
				endConnection();
				return DistributedExecutionManager::DistributedCommunication::FAILURE;
            }
		}
	return DistributedExecutionManager::DistributedCommunication::NOTHING;
}

void DistributedExecutionManager::createNewConnection(int socket) {
	_model->getTracer()->traceError(TraceManager::Level::L5_event, "Network event. Starting a new connection with a client.");
	fds[1].fd = socket;
	fds[1].events = POLLIN;
	nfds++;
}

//Server
void DistributedExecutionManager::endConnection() {
	_model->getTracer()->traceError(TraceManager::Level::L5_event, "Network event. Ending a connection with the client.");
	close(fds[1].fd);
	fds[1].fd = 0;
	nfds--;
}

void DistributedExecutionManager::startServerSimulation() {
	DistributedExecutionManager::SocketData* socketData = createNewSocketDataServer(6000);
	createServerBind(socketData);
	createServerListen(socketData);
	std::thread thread_simulation;
	std::thread thread_results;

	// _networkGetResult.lock();
	// _networkSimulationEnded.lock();
	//thread_results = std::thread(&ModelSimulation::threadWaitResults, this, socketData);
	while (1) {
		DistributedExecutionManager::DistributedCommunication code = getNextDistributedCommunicationCode();
		switch (code)
		{
			case DistributedExecutionManager::DistributedCommunication::NOTHING:
				break;
			case DistributedExecutionManager::DistributedCommunication::INIT_CONNECTION:
				sendCodeMessage(DistributedExecutionManager::DistributedCommunication::INIT_CONNECTION);
				break;
			case DistributedExecutionManager::DistributedCommunication::BENCHMARK:
				sendBenchmark();
				break;
			case DistributedExecutionManager::DistributedCommunication::SEND_MODEL:
				if (!receivePayload())
					break;
				_model->getSimulation()->setNumberOfReplications(socketData->_replicationNumber);
				// setSeed(socketData->_seed);
                if (_model->load("networkModel.gen")) {
                    if (thread_simulation.joinable())
                        thread_simulation.join();
                    // _isNetworkModelRunningMutex.lock();
                    // _isNetworkModelRunning = true;
                    // _isNetworkModelRunningMutex.unlock();
                    // thread_simulation = std::thread(&ModelSimulation::start, this);
                    sendCodeMessage(DistributedExecutionManager::DistributedCommunication::SEND_MODEL);
                } else {
                    sendCodeMessage(DistributedExecutionManager::DistributedCommunication::FAILURE);
                }
				break;
			case DistributedExecutionManager::DistributedCommunication::RESULTS:
				// if (_isNetworkModelRunning) {
				// 	_getResultNetworkMutex.lock();
				// 	_getResultNetwork = true;
				// 	_getResultNetworkMutex.unlock();
				// }
				break;
			case DistributedExecutionManager::DistributedCommunication::FAILURE:
				break;
			default:
				break;
		}
	}
}

bool DistributedExecutionManager::receivePayload() {
    char buffer[sizeof(DistributedExecutionManager::ModelExecutionPayload)];
    
    while (1) {
		int p = recv(fds[1].fd, buffer, sizeof(DistributedExecutionManager::ModelExecutionPayload), 0);
		std::cout << p << std::endl;
		if (p == 0) {
			break;
		}
		if (p > 0) {
            break;
		}
		if (p < 1) {
			_model->getTracer()->traceError(TraceManager::Level::L1_errorFatal, "Fail to receive the model.");
			endConnection();
			return false;
		}
    }
}

bool DistributedExecutionManager::sendPayload(SocketData* socketData) {
    char buffer[sizeof(DistributedExecutionManager::ModelExecutionPayload)];
	send(socketData->_socket , buffer , sizeof(DistributedExecutionManager::ModelExecutionPayload) , 0 );
	if (receiveCodeMessage(DistributedCommunication::SEND_MODEL, socketData->_socket)) {
		_model->getTracer()->traceError(TraceManager::Level::L5_event, "sendModel OK!");
	} else {
		_model->getTracer()->traceError(TraceManager::Level::L5_event, "sendModel ERROR!");
	}
}

void DistributedExecutionManager::createClientThreadTask(SocketData* socketData) {
    sendPayload(socketData);
}

void DistributedExecutionManager::startClientSimulation() {
	// if (_replica == 1) {
	// 	_model->getTracer()->traceError(Util::TraceLevel::L4_warning, "There is only one replication defined. Network simulation will be ignored.");
	// 	start();
	// 	return;

	// }
	// if (!_network->check()) {
	// 	_model->getTracer()->traceError(Util::TraceLevel::L1_errorFatal, "Network check failed. Cannot start simulation in network.");
	// 	return;
	// }
    connectToServers();
	//getBenchmarks(&_network->_sockets);
	//_networkReplicationMutex.lock();
	// _networkMutex.lock();
	//_networkScheduler->set(_model->getSimulation()->_numberOfReplications, &_network->_sockets);
	//Create a thread for every socket...
	int threads_size = 1;
	std::thread thread_simulation[threads_size];
	std::thread thread_start;
	//thread_start = std::thread(&ModelSimulation::start, this);
	for (int i = 0; i < threads_size; i++) {
		thread_simulation[i] = std::thread(&DistributedExecutionManager::createClientThreadTask, this, _sockets.at(i));
	}

	// start();
	thread_start.join();
	for (int i = 0; i < threads_size; i++)
		thread_simulation[i].detach();
	for (int i = 0; i < _sockets.size(); i++)
		close(_sockets.at(i)->_socket);
}

void DistributedExecutionManager::sendBenchmark() {
	int client_socket = fds[1].fd;
	char* tmp = reinterpret_cast<char*>(&_benchmarkInfo);
	send(client_socket, tmp, sizeof(Benchmark::BenchmarkInfo), 0);
}