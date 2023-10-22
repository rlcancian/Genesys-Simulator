#include <iostream>
#include <thread>
#include <netinet/in.h>
#include <poll.h>
#include <arpa/inet.h>
#include <fstream>
#include <sstream>
#include <future>
#include <chrono>


#include "DistributedExecutionManager.h"
#include "../../kernel/simulator/TraceManager.h"
#include "../../kernel/TraitsKernel.h"

uint64_t htonll(uint64_t value) {
    if (__BYTE_ORDER == __BIG_ENDIAN) {
        return value;
    } else if (__BYTE_ORDER == __LITTLE_ENDIAN) {
        uint32_t high_part = htonl((uint32_t)(value >> 32));
        uint32_t low_part = htonl((uint32_t)(value & 0xFFFFFFFFLL));
        return ((uint64_t)low_part << 32) | high_part;
    }
}

uint64_t ntohll(uint64_t value) {
    if (__BYTE_ORDER == __BIG_ENDIAN) {
        return value;
    } else if (__BYTE_ORDER == __LITTLE_ENDIAN) {
        uint32_t high_part = ntohl((uint32_t)(value >> 32));
        uint32_t low_part = ntohl((uint32_t)(value & 0xFFFFFFFFLL));
        return ((uint64_t)low_part << 32) | high_part;
    }
}

DistributedExecutionManager::DistributedExecutionManager(Model* model) {
    originalNumberOfReplications = model->getSimulation()->getNumberOfReplications();
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
	// HARDCODED
	std::vector<int> portsList;
	portsList.insert(portsList.end(), 6666);
	//portsList.insert(portsList.end(), 6667);


	for (int i = 0; i <  portsList.size(); i++) {
		//server ip
		DistributedExecutionManager::SocketData* tempSocketData = createNewSocketDataClient(6000, 0);
		_model->getTracer()->traceError(TraceManager::Level::L1_errorFatal, "[DEM - EVENT] Executing connectToServers");

		if (connectToServerSocket(tempSocketData)) {
			sendCodeMessage(DistributedCommunication::INIT_CONNECTION, tempSocketData->_socket);
			_model->getTracer()->traceError(TraceManager::Level::L1_errorFatal, "[EVENT] Sending INIT_CONNECTION to server");
			
			if (receiveCodeMessage(DistributedCommunication::INIT_CONNECTION, tempSocketData->_socket)) {
				_model->getTracer()->traceError(TraceManager::Level::L1_errorFatal, "[EVENT] INIT_CONNECTION received, adding to socket list");

				// Connection has been established - add info about socket and add it to list
				// socketDataItem still waiting for number of replications
				tempSocketData->_id = i;
				tempSocketData->_seed = getRandomSeed();
				tempSocketData->_replicationNumber = -1;

				appendSocketDataList(tempSocketData);
				_model->getTracer()->traceError(TraceManager::Level::L1_errorFatal, "[EVENT] Info about thread that is being added");
				_model->getTracer()->traceError(TraceManager::Level::L1_errorFatal, socketDataToString(tempSocketData));
				// breaks loop
				return true;
			} else {
				_model->getTracer()->traceError(TraceManager::Level::L1_errorFatal, "[ERROR - FATAL] Instance didn't return the expected command");
				return false;
			}
		} else {
			_model->getTracer()->traceError(TraceManager::Level::L1_errorFatal, "[ERROR - FATAL] Connection couldn't be established with instance");
			return false;
		}
	}
	return true;
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

DistributedExecutionManager::SocketData* DistributedExecutionManager::createClientSocketData(int clientSockfd, const struct sockaddr_in& clientAddress) {
    DistributedExecutionManager::SocketData* socketData = new DistributedExecutionManager::SocketData();
	socketData->_id = -1;
	socketData->_seed = -1;
	socketData->_replicationNumber = -1;
	socketData->_socket = clientSockfd;
	socketData->_address = clientAddress;
    return socketData;
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
					DistributedCommunication codeEnum = static_cast<DistributedCommunication>(code);
					//TEMPORARY
					std::cout << codeEnum << "\n";
					_model->getTracer()->traceError(TraceManager::Level::L5_event, "Network event. Client send a code. -> " + enumToString(codeEnum));
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

// Create simpler start simulation code to get it working and then add socket managment
// create the optimal path for server threads to pass
// create attribute bool for client or server
// create semaphore for execution -> if semaphore is occupied then return busy

void DistributedExecutionManager::startServerSimulation() {
	struct sockaddr_in clientAddress;
	socklen_t clientLen = sizeof(clientAddress);
    std::vector<std::future<DistributedExecutionManager::ResultPayload>> futureThreads;
	DistributedExecutionManager::SocketData* socketData = createNewSocketDataServer(6000);
	
	createServerBind(socketData);
	createServerListen(socketData);

	_model->getTracer()->traceError(TraceManager::Level::L5_event, "[DEM] Entering listening loop");

	while(1) {
		int clientSockFd = accept(socketData->_socket, (struct sockaddr *)&clientAddress, &clientLen);
		_model->getTracer()->traceError(TraceManager::Level::L1_errorFatal, socketInfoToString(clientSockFd, clientAddress, clientLen));
		if (clientSockFd < 0) {
			_model->getTracer()->traceError(TraceManager::Level::L1_errorFatal, "[DEM] Error accepting connection");
            continue;
		}

		SocketData* clientSocketData = createClientSocketData(clientSockFd, clientAddress);
		futureThreads.push_back(std::async(std::launch::async, &DistributedExecutionManager::createServerThreadTask, this, clientSocketData));

	}

}

// void DistributedExecutionManager::startServerSimulation() {
// 	DistributedExecutionManager::SocketData* socketData = createNewSocketDataServer(6000);
// 	createServerBind(socketData);
// 	createServerListen(socketData);
// 	std::thread thread_simulation;
// 	std::thread thread_results;

// 	//thread_results = std::thread(&ModelSimulation::threadWaitResults, this, socketData);
// 	_model->getTracer()->traceError(TraceManager::Level::L5_event, "Entering listening loop");
// 	while (1) {
// 		DistributedExecutionManager::DistributedCommunication code = getNextDistributedCommunicationCode();
// 		switch (code)
// 		{
// 			case DistributedExecutionManager::DistributedCommunication::NOTHING:
// 				_model->getTracer()->traceError(TraceManager::Level::L5_event, "Received NOTHING from client");
// 				break;
// 			case DistributedExecutionManager::DistributedCommunication::INIT_CONNECTION:
// 				_model->getTracer()->traceError(TraceManager::Level::L5_event, "Received INIT_CONNECTION from client");
// 				sendCodeMessage(DistributedExecutionManager::DistributedCommunication::INIT_CONNECTION);
// 				break;
// 			case DistributedExecutionManager::DistributedCommunication::BENCHMARK:
// 				_model->getTracer()->traceError(TraceManager::Level::L5_event, "Received BENCHMARK from client");
// 				sendBenchmark();
// 				break;
// 			case DistributedExecutionManager::DistributedCommunication::SEND_MODEL:
// 				_model->getTracer()->traceError(TraceManager::Level::L5_event, "Received SEND_MODEL from client");
// 				if (!receiveRequestPayload(socketData)) {
// 					break;
// 				} else {
// 					_model->getTracer()->traceError(TraceManager::Level::L5_event, "receiveRequestPayload completed!");
// 				} 
// 				// _model->getSimulation()->setNumberOfReplications(socketData->_replicationNumber);
// 				// if (thread_simulation.joinable())
// 				// 	thread_simulation.join();

// 				sendCodeMessage(DistributedExecutionManager::DistributedCommunication::SEND_MODEL);
// 				break;
// 			case DistributedExecutionManager::DistributedCommunication::RESULTS:
// 				_model->getTracer()->traceError(TraceManager::Level::L5_event, "Received RESULTS from client");
// 				break;
// 			case DistributedExecutionManager::DistributedCommunication::FAILURE:
// 				break;
// 			default:
// 				break;
// 		}
// 	}
// }

bool DistributedExecutionManager::receiveSocketData(SocketData* socketData) {
	char buffer[sizeof(DistributedExecutionManager::SocketData)];
	//SocketData* socketDataTemp;
	int res = recv(socketData->_socket, &buffer, sizeof(DistributedExecutionManager::SocketData), 0);
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

bool DistributedExecutionManager::receiveFileSize(SocketData* socketData, int& fileSize) {
    int networkByteOrderFileSize;

    int res = recv(socketData->_socket, &networkByteOrderFileSize, sizeof(networkByteOrderFileSize), 0);

    _model->getTracer()->traceError(TraceManager::Level::L1_errorFatal, 
                                    "[DEM] Size of received data: " + std::to_string(res) + 
                                    ", Size of int variable: " + std::to_string(sizeof(networkByteOrderFileSize)));

    if (res == sizeof(networkByteOrderFileSize)) {
        _model->getTracer()->traceError(TraceManager::Level::L5_event, "[DEM] fileSize received");
        return true;
    } else {
		perror("fileSize failed, here why:");
        _model->getTracer()->traceError(TraceManager::Level::L1_errorFatal, "[DEM] Failed to receive fileSize");
        return false;
    }
}

bool DistributedExecutionManager::receiveRequestPayload(SocketData* socketData) {
	if (!receiveSocketData(socketData)) { closeConnection(); return false; };

	int fileSize;
	receiveFileSize(socketData, fileSize);
	_model->getTracer()->traceError(TraceManager::Level::L1_errorFatal, "fileSize: " + std::to_string(fileSize));


	if (fileSize < sizeof(uint64_t)) {
		closeConnection();
		return false;
	}
	_model->getTracer()->traceError(TraceManager::Level::L1_errorFatal, "Failed but continuing to model");

	
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

bool DistributedExecutionManager::sendSocketData(SocketData* socketData) {
	char buffer[sizeof(DistributedExecutionManager::SocketData)];
	char emptyBuffer[sizeof(DistributedExecutionManager::SocketData)];

	std::memcpy(buffer, socketData, sizeof(DistributedExecutionManager::SocketData));
    int bytesSent = send(socketData->_socket, buffer, sizeof(DistributedExecutionManager::SocketData), 0);

	if (std::memcmp(buffer, emptyBuffer, sizeof(DistributedExecutionManager::SocketData)) == 0) {
		_model->getTracer()->traceError(TraceManager::Level::L5_event, "bufferVazio");
		return false;
	}

    if (bytesSent != sizeof(buffer)) {
		_model->getTracer()->traceError(TraceManager::Level::L1_errorFatal, "Couldn't send socketData");
		return false;
    } else {
		std::ostringstream oss;
		oss << "[DEM] Sent socketData bytes: " << bytesSent;
		_model->getTracer()->traceError(TraceManager::Level::L5_event, oss.str());
		return true;
    }
}

bool DistributedExecutionManager::sendFileSize(SocketData* socketData, int fileSize) {
	int res = send(socketData->_socket, &fileSize, sizeof(fileSize), 0);
    if (res == sizeof(fileSize)) {
        _model->getTracer()->traceError(TraceManager::Level::L5_event, "[DEM] fileSize sent");
		return true;
    } else {
        _model->getTracer()->traceError(TraceManager::Level::L1_errorFatal, "[DEM] Failed to send fileSize");
		return false;
    }
}

bool DistributedExecutionManager::sendRequestPayload(SocketData* socketData) {
	std::string fileToSend = modelToFile();
    
    // Get the file size
    std::ifstream file("modeloEnvio.gen", std::ios::binary | std::ios::ate);
    uint64_t fileSize = file.tellg();
	fileSize = static_cast<int>(fileSize);
	_model->getTracer()->traceError(TraceManager::Level::L5_event, "[DEM] fileSize real size: " + std::to_string(fileSize));

    file.close();
        
    // Send the SocketData with the file size
    if (!sendSocketData(socketData)) {
		return false;
	};

	// Send the file size
    if (!sendFileSize(socketData, fileSize)) {
		return false;
	}

	_model->getTracer()->traceError(TraceManager::Level::L5_event, "[DEM] Finished sending fileSize, trying to send model");
    // Send the actual file data
    if (send(socketData->_socket, fileToSend.c_str(), fileToSend.length(), 0) < 0) {
		return false;
	}

    if (receiveCodeMessage(DistributedCommunication::SEND_MODEL, socketData->_socket)) {
        _model->getTracer()->traceError(TraceManager::Level::L5_event, "[DEM] sendModel OK!");
		return true;
    } else {
        _model->getTracer()->traceError(TraceManager::Level::L1_errorFatal, "[DEM] sendModel ERROR!");
		return false;
    }
}

bool DistributedExecutionManager::sendResultPayload(SocketData* socketData) {
	
	// MOCK RESULT
	ResultPayload response;
	response.code = DistributedCommunication::RESULTS;
	std::vector<double> results;
	results.push_back(5);
	results.push_back(10);
	results.push_back(1000);

	char buffer[sizeof(ResultPayload)];
	std::memcpy(buffer, &response, sizeof(ResultPayload));
	int bytesSent = send(socketData->_socket, buffer, sizeof(buffer), 0);
	
	if (bytesSent != sizeof(buffer)) {
		_model->getTracer()->traceError(TraceManager::Level::L1_errorFatal, "[DEM] sendResultPayload error!");
		return false;
	}

	return true;

}

bool DistributedExecutionManager::receiveResultPayload(SocketData* socketData, ResultPayload& resultPayload) {
	char buffer[sizeof(ResultPayload)];
	int bytesReceived = recv(socketData->_socket, &buffer, sizeof(buffer), 0);

	if (bytesReceived == sizeof(buffer)) {
		_model->getTracer()->traceError(TraceManager::Level::L5_event, "[DEM] receiveResultPayload finished!");
		std::memcpy(&resultPayload, buffer, sizeof(ResultPayload));
		return true;
	} else {
		_model->getTracer()->traceError(TraceManager::Level::L1_errorFatal, "[DEM] receiveResultPayload error!");
		return false;
	}
}
// Create semaphores for when receiving results from threads or sending the model
DistributedExecutionManager::ResultPayload DistributedExecutionManager::createClientThreadTask(SocketData* socketData) {
	ResultPayload resultPayload;
	
	// sendCodeMessage(DistributedCommunication::INIT_CONNECTION, socketData->_socket);
	// if (receiveCodeMessage(DistributedCommunication::INIT_CONNECTION, socketData->_socket)) {
		// sempahore lock for sending payload
	//_model->getTracer()->traceError(TraceManager::Level::L5_event, "[DEM] Handshake with server");
	// semaphore release and let thread do the calculations -> NOT NEEDED
	if (!sendRequestPayload(socketData)) {
		resultPayload.code = DistributedCommunication::FAILURE;
		return resultPayload;
	}

	// wait for results
	if (!receiveResultPayload(socketData, resultPayload) || resultPayload.code != DistributedCommunication::RESULTS) {
		resultPayload.code = DistributedCommunication::FAILURE;
	}

	return resultPayload;

		// client thread wait on a send of "receiving results"
		// client threads consumes code and locks semaphore for results
	// } else {
	// 	resultPayload.code = DistributedCommunication::FAILURE;
	// 	return resultPayload;
	// }
	
	// sendCodeMessage(DistributedCommunication::SEND_MODEL, socketData->_socket);
	// if (receiveCodeMessage(DistributedCommunication::SEND_MODEL), socketData->_socket) {
	// 	sendRequestPayload(socketData);
	// }
}

DistributedExecutionManager::ResultPayload DistributedExecutionManager::createServerThreadTask(SocketData* socketData) {
	ResultPayload resultPayload;

	// if (receiveCodeMessage(DistributedCommunication::INIT_CONNECTION, socketData->_socket)) {
	// 	sendCodeMessage(DistributedCommunication::INIT_CONNECTION, socketData->_socket);
		//_model->getTracer()->traceError(TraceManager::Level::L5_event, "[DEM] Handshake with client");

		// getting ready to send stuff
	if (receiveCodeMessage(DistributedCommunication::INIT_CONNECTION, socketData->_socket)) {
		sendCodeMessage(DistributedCommunication::INIT_CONNECTION, socketData->_socket);

		if (!receiveRequestPayload(socketData)) {
			resultPayload.code = DistributedCommunication::FAILURE;
			return resultPayload;
		}

		// while loop to check if client thread wants to cancel simulation
		// execute model simulation -> semaphore for using the simulator
		//_model->getSimulation()->start();
		//send results

		if (!sendResultPayload(socketData)) {
			resultPayload.code = DistributedCommunication::FAILURE;
			return resultPayload;
		}
	} else {
		resultPayload.code = DistributedCommunication::FAILURE;
		return resultPayload;
	}
	// }

	// return resultPayload;

}

void DistributedExecutionManager::startClientSimulation() {
	// create semaphores
	
	connectToServers();
	_model->getTracer()->traceError(TraceManager::Level::L5_event, "[DEM] Connect servers done!");


	int threadNumber = _sockets.size();
	int hardwareThreadNumber = getNumberThreads();

	if (threadNumber > hardwareThreadNumber) {
		threadNumber = hardwareThreadNumber;
	}

	int repsByThread = _model->getSimulation()->getNumberOfReplications()/threadNumber;
	//_model->getSimulation()->setNumberOfReplications(repsByThread);

	std::vector<std::future<DistributedExecutionManager::ResultPayload>> threads;
    std::unordered_map<std::future<DistributedExecutionManager::ResultPayload>*, std::chrono::time_point<std::chrono::steady_clock>> startTimes;

	// resultado = future(fazerAlgo())

	for (int i = 0; i < threadNumber; i++) {
		_sockets[i]->_replicationNumber = repsByThread;
		threads.push_back(std::async(std::launch::async, &DistributedExecutionManager::createClientThreadTask, this, _sockets[i]));
		startTimes[&threads.at(i)] = std::chrono::steady_clock::now();
	}

	// set a timetout
	auto timeout = std::chrono::seconds(20);


	// Use a non-blocking loop to periodically check the status of the futures
    while (!threads.empty()) {
        for (auto it = threads.begin(); it != threads.end();) {
            auto& future = *it;
            auto status = future.wait_for(std::chrono::milliseconds(0));

            if (status == std::future_status::ready) {
                // If the future is ready, retrieve the result
				_model->getTracer()->traceError(TraceManager::Level::L5_event, "[DEM] thread ready!, removing it!");
                ResultPayload result = future.get();
                it = threads.erase(it);

				_model->getTracer()->traceError(TraceManager::Level::L5_event, resultPayloadtoString(&result));
            } else {
                // Check the elapsed time for the future
                auto elapsed = std::chrono::steady_clock::now() - startTimes[&future];

                if (elapsed > timeout) {
                    // If the future has exceeded the timeout, cancel the operation and erase the future
					_model->getTracer()->traceError(TraceManager::Level::L5_event, "[DEM] TIMEOUT for thread, removing it!");
					// add here a list of sockets to try again (?)
                    it = threads.erase(it);
                } else {
                    ++it;
                }
            }
        }
	}
	//destroy semaphores

	return;
}

void DistributedExecutionManager::sendBenchmark() {
	int client_socket = fds[1].fd;
	char* tmp = reinterpret_cast<char*>(&_benchmarkInfo);
	send(client_socket, tmp, sizeof(Benchmark::BenchmarkInfo), 0);
}

std::string DistributedExecutionManager::socketDataToString(DistributedExecutionManager::SocketData* data) {
    std::ostringstream oss;
    oss << "ID: " << data->_id << "\n"
        << "Seed: " << data->_seed << "\n"
        << "Replication Number: " << data->_replicationNumber << "\n"
        << "Address: " << inet_ntoa(data->_address.sin_addr) << "\n"
        << "Port: " << ntohs(data->_address.sin_port) << "\n"
        << "Socket: " << data->_socket << "\n";
    return oss.str();
}

std::string DistributedExecutionManager::resultPayloadtoString(DistributedExecutionManager::ResultPayload* payload) {
    std::ostringstream oss;
    oss << "Code: " << enumToString(static_cast<DistributedExecutionManager::DistributedCommunication>(payload->code)) << "\n"
        << "ThreadId: " << payload->threadId << "\n"
        << "Results: ";
    for (const auto& result : payload->results) {
        oss << result << " ";
    }
    oss << "\n";
    return oss.str();
}

std::string DistributedExecutionManager::enumToString(DistributedCommunication code) const {
    switch (code) {
        case DistributedCommunication::INIT_CONNECTION:
            return "INIT_CONNECTION";
        case DistributedCommunication::SEND_MODEL:
            return "SEND_MODEL";
        case DistributedCommunication::RECEIVE_DATA:
            return "RECEIVE_DATA";
        case DistributedCommunication::RESULTS:
            return "RESULTS";
        case DistributedCommunication::CLOSE_CONNECTION:
            return "CLOSE_CONNECTION";
        case DistributedCommunication::NOTHING:
            return "NOTHING";
        case DistributedCommunication::FAILURE:
            return "FAILURE";
        case DistributedCommunication::BENCHMARK:
            return "BENCHMARK";
        default:
            return "UNKNOWN";
    }
}

std::string DistributedExecutionManager::socketInfoToString(int client_sockfd, const struct sockaddr_in& clientAddress, socklen_t clientLen) {
    std::ostringstream oss;

    // Converta o endereço IP do cliente de um número para uma string
    char clientIP[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(clientAddress.sin_addr), clientIP, INET_ADDRSTRLEN);

    // Obtenha o número da porta do cliente
    int clientPort = ntohs(clientAddress.sin_port);

    // Crie uma string contendo as informações
    oss << "Client Socket FD: " << client_sockfd << "\n"
        << "Client IP: " << clientIP << "\n"
        << "Client Port: " << clientPort << "\n"
        << "Client Length: " << clientLen;

    return oss.str();
}