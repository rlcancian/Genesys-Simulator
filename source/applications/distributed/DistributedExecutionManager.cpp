#include <thread>
#include <netinet/in.h>
#include <poll.h>
#include <arpa/inet.h>
#include <fstream>

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
    DistributedExecutionManager::SocketData* tempSocketData = createNewSocketDataClient(6000, 0);
	_model->getTracer()->traceError(TraceManager::Level::L1_errorFatal, "[EVENT] Executing connect to servers");

    if (connectToServerSocket(tempSocketData)) {
        sendCodeMessage(DistributedCommunication::INIT_CONNECTION, tempSocketData->_socket);
		_model->getTracer()->traceError(TraceManager::Level::L1_errorFatal, "[EVENT] Sending init_connection to server");
        if (receiveCodeMessage(DistributedCommunication::INIT_CONNECTION, tempSocketData->_socket)) {
			_model->getTracer()->traceError(TraceManager::Level::L1_errorFatal, "[EVENT] init_connection received");
            appendSocketDataList(tempSocketData);
			return true;
			_model->getTracer()->traceError(TraceManager::Level::L1_errorFatal, "Added to socketDataList");
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
    socketData->_address.sin_addr.s_addr = inet_addr("127.0.0.1");
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
	if (setsockopt(socketData->_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on)) < 0) {
    	getModel()->getTracer()->traceError(TraceManager::Level::L1_errorFatal, "setsockopt falhou - Socket ocupado?");
	}

	fds[0].fd = socketData->_socket;
	return socketData;
}


bool DistributedExecutionManager::connectToServerSocket(DistributedExecutionManager::SocketData* socketData) {
	getModel()->getTracer()->traceError(TraceManager::Level::L5_event, "[EVENT] Attempting to connect to server socket.");
	if ((connect(socketData->_socket, (struct sockaddr *) &socketData->_address, sizeof(socketData->_address))) < 0) {
		getModel()->getTracer()->traceError(TraceManager::Level::L1_errorFatal, "[ERROR - FATAL] Failed to connect to the server.");
		perror("connect failed, heres why");
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
	_model->getTracer()->traceError(TraceManager::Level::L5_event, "Polling for events");
	poll(fds, nfds, -1);
	_model->getTracer()->traceError(TraceManager::Level::L5_event, "Poll successful, getting ready read input");

	int buffer = -1;

	for (int i = 0; i < nfds; i++) {
		//_model->getTracer()->traceError(TraceManager::Level::L5_event, std::to_string(fds[i].revents));
		switch (fds[i].revents) {
			case 0:
				_model->getTracer()->traceError(TraceManager::Level::L5_event, "revents is 0 breaking loop");
				break;
			case POLLIN:
			{
				if (fds[i].fd == fds[0].fd) {
					struct sockaddr_in addr;
					int socklen, newsock, bytes, ret;
					char op, str[INET_ADDRSTRLEN];
					_model->getTracer()->traceError(TraceManager::Level::L5_event, "Server will be accepting");
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
					_model->getTracer()->traceError(TraceManager::Level::L5_event, "Sending INIT_CONNECTION to client");
					return DistributedExecutionManager::DistributedCommunication::INIT_CONNECTION;
				}
				_model->getTracer()->traceError(TraceManager::Level::L5_event, "Using buffer to get code");
				int p = recv(fds[i].fd, &buffer, sizeof(int), 0);
				_model->getTracer()->traceError(TraceManager::Level::L5_event, "Code is in buffer");
				if (p > 0) {
					DistributedExecutionManager::DistributedCommunication code;
					code = (DistributedExecutionManager::DistributedCommunication)ntohl(buffer);
					_model->getTracer()->traceError(TraceManager::Level::L5_event, "Network event. Client send a code.");
					std::cout << (int)code << std::endl;
					fflush(stdout);
					return code;
				} else {
					closeConnection();
					DistributedExecutionManager::DistributedCommunication::NOTHING;
				}
				break;
			}
			default:
				_model->getTracer()->traceError(TraceManager::Level::L4_warning, "Network error. Unespected revents.");
				closeConnection();
				return DistributedExecutionManager::DistributedCommunication::FAILURE;
            }
		}
	_model->getTracer()->traceError(TraceManager::Level::L5_event, "Returning nothing from getNetwork");
	return DistributedExecutionManager::DistributedCommunication::NOTHING;
}

void DistributedExecutionManager::createNewConnection(int socket) {
	_model->getTracer()->traceError(TraceManager::Level::L5_event, "Network event. Starting a new connection with a client.");
	fds[1].fd = socket;
	fds[1].events = POLLIN;
	_model->getTracer()->traceError(TraceManager::Level::L5_event, "Adding to nfds " + std::to_string(nfds) + " to " + std::to_string(nfds + 1));
	nfds++;
}


void DistributedExecutionManager::closeConnection() {
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

	//thread_results = std::thread(&ModelSimulation::threadWaitResults, this, socketData);
	_model->getTracer()->traceError(TraceManager::Level::L5_event, "Entering listening loop");
	while (1) {
		DistributedExecutionManager::DistributedCommunication code = getNextDistributedCommunicationCode();
		switch (code)
		{
			case DistributedExecutionManager::DistributedCommunication::NOTHING:
				_model->getTracer()->traceError(TraceManager::Level::L5_event, "Received NOTHING from client");
				break;
			case DistributedExecutionManager::DistributedCommunication::INIT_CONNECTION:
				_model->getTracer()->traceError(TraceManager::Level::L5_event, "Received INIT_CONNECTION from client");
				sendCodeMessage(DistributedExecutionManager::DistributedCommunication::INIT_CONNECTION);
				break;
			case DistributedExecutionManager::DistributedCommunication::BENCHMARK:
				_model->getTracer()->traceError(TraceManager::Level::L5_event, "Received BENCHMARK from client");
				sendBenchmark();
				break;
			case DistributedExecutionManager::DistributedCommunication::SEND_MODEL:
				_model->getTracer()->traceError(TraceManager::Level::L5_event, "Received SEND_MODEL from client");
				if (!receivePayload(socketData)) {
					break;
				} else {
					_model->getTracer()->traceError(TraceManager::Level::L5_event, "receivePayload completed!");
				} 
				_model->getSimulation()->setNumberOfReplications(socketData->_replicationNumber);
				if (thread_simulation.joinable())
					thread_simulation.join();

				sendCodeMessage(DistributedExecutionManager::DistributedCommunication::SEND_MODEL);
				break;
			case DistributedExecutionManager::DistributedCommunication::RESULTS:
				_model->getTracer()->traceError(TraceManager::Level::L5_event, "Received RESULTS from client");
				break;
			case DistributedExecutionManager::DistributedCommunication::FAILURE:
				break;
			default:
				break;
		}
	}
}

bool DistributedExecutionManager::receiveSocketData(SocketData* socketData) {
	char buffer[sizeof(DistributedExecutionManager::SocketData)];
	//SocketData* socketDataTemp;
	int res = recv(fds[1].fd, &buffer, sizeof(DistributedExecutionManager::SocketData), 0);
	if (res > 0) {
		std::memcpy(socketData, buffer, sizeof(SocketData));
		_model->getTracer()->traceError(TraceManager::Level::L5_event, "socketData received.");
		return true;
	}
	if (res < 1) {
		_model->getTracer()->traceError(TraceManager::Level::L1_errorFatal, "Fail to receive socketData.");
		return false;
	}
	return false;
}

uint64_t DistributedExecutionManager::receiveFileSize(SocketData* socketData) {
	uint64_t fileSize;
    int res = recv(socketData->_socket, &fileSize, sizeof(fileSize), 0) < sizeof(fileSize);
	if (res > 0) {
		_model->getTracer()->traceError(TraceManager::Level::L5_event, "fileSize received.");
		return fileSize;
	}
	if (res < 1) {
		_model->getTracer()->traceError(TraceManager::Level::L1_errorFatal, "Fail to receive fileSize.");
		perror("ERROR ->");
		return -1;
	}
	return -1;
}

bool DistributedExecutionManager::receivePayload(SocketData* socketData) {
	if (!receiveSocketData(socketData)) { closeConnection(); return false; };

	uint64_t fileSize = receiveFileSize(socketData);

	if (fileSize == -1) { closeConnection(); return false; }
	
	std::ofstream file;
	file.open("networkModel.gen", std::ios::out | std::ios::binary);
	char* buffer = new char[fileSize];

	if (!file.is_open()) {
		_model->getTracer()->traceError(TraceManager::Level::L1_errorFatal, "Fail to open a file.");
		closeConnection();
		return false;
		}

	ssize_t totalBytesReceived = 0;
	while (totalBytesReceived < fileSize) {
		ssize_t bytesReceived = recv(socketData->_socket, buffer + totalBytesReceived, fileSize - totalBytesReceived, 0);
		if (bytesReceived < 0) {
			_model->getTracer()->traceError(TraceManager::Level::L1_errorFatal, "Unexpected error in receiving model");
			closeConnection();
			delete[] buffer;
			return false;
		} else if (bytesReceived == 0) {
			_model->getTracer()->traceError(TraceManager::Level::L1_errorFatal, "Unexpected error. No bytes received");
			closeConnection();
			delete[] buffer;
			return false;
		}
		totalBytesReceived += bytesReceived;
	}

    file.write(buffer, fileSize);
    if (!file.good()) {
		closeConnection();
        delete[] buffer;
        return false;
    }

    file.close();
    delete[] buffer;
    return true;
}

std::string DistributedExecutionManager::modelToFile() {
	std::fstream file;
	_model->save("modeloEnvio.gen");
	file.open("modeloEnvio.gen", std::ios::in | std::ios::binary);
	std::string contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	return contents;
}

void DistributedExecutionManager::sendSocketData(SocketData* socketData) {
	char buffer[sizeof(DistributedExecutionManager::SocketData)];
	char emptyBuffer[sizeof(DistributedExecutionManager::SocketData)];

	std::memcpy(buffer, socketData, sizeof(DistributedExecutionManager::SocketData));
    send(socketData->_socket, buffer, sizeof(DistributedExecutionManager::SocketData), 0);

	if (std::memcmp(buffer, emptyBuffer, sizeof(DistributedExecutionManager::SocketData)) == 0) {
		_model->getTracer()->traceError(TraceManager::Level::L5_event, "bufferVazio");
	}
}

bool DistributedExecutionManager::sendFileSize(SocketData* socketData, uint64_t fileSize) {
    int res = send(socketData->_socket, &fileSize, sizeof(fileSize), 0);
    if (res > 0) {
        _model->getTracer()->traceError(TraceManager::Level::L5_event, "fileSize sent.");
    } else {
        _model->getTracer()->traceError(TraceManager::Level::L1_errorFatal, "Fail to send fileSize.");
        perror("ERROR ->");
    }
}

bool DistributedExecutionManager::sendPayload(SocketData* socketData) {
    std::string fileToSend = modelToFile();
    
    // Get the file size
    std::ifstream file("modeloEnvio.gen", std::ios::binary | std::ios::ate);
    uint64_t fileSize = file.tellg();
    file.close();
        
    // Send the SocketData with the file size
    sendSocketData(socketData);

	// Send the file size
    sendFileSize(socketData, fileSize);

    // Send the actual file data
    send(socketData->_socket, fileToSend.c_str(), fileToSend.length(), 0);

    if (receiveCodeMessage(DistributedCommunication::SEND_MODEL, socketData->_socket)) {
        _model->getTracer()->traceError(TraceManager::Level::L5_event, "sendModel OK!");
    } else {
        _model->getTracer()->traceError(TraceManager::Level::L5_event, "sendModel ERROR!");
    }
}

void DistributedExecutionManager::createClientThreadTask(SocketData* socketData) {
	sendCodeMessage(DistributedCommunication::SEND_MODEL, socketData->_socket);
	if (receiveCodeMessage(DistributedCommunication::SEND_MODEL), socketData->_socket) {
		sendPayload(socketData);
	}
}

void DistributedExecutionManager::startClientSimulation() {
	connectToServers();
	int threadNumber = 1;
	std::thread threadSimulation[threadNumber];
	//thread_start = std::thread(&ModelSimulation::start, this);
	for (int i = 0; i < threadNumber; i++) {
		threadSimulation[i] = std::thread(&DistributedExecutionManager::createClientThreadTask, this, _sockets.at(i));
	}

	for (int i = 0; i < threadNumber; i++)
		threadSimulation[i].detach();
	for (int i = 0; i < _sockets.size(); i++)
		close(_sockets.at(i)->_socket);
}

void DistributedExecutionManager::sendBenchmark() {
	int client_socket = fds[1].fd;
	char* tmp = reinterpret_cast<char*>(&_benchmarkInfo);
	send(client_socket, tmp, sizeof(Benchmark::BenchmarkInfo), 0);
}