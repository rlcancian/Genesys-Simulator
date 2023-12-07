#include <cstring> // for strerror
#include <iostream>
#include <thread>
#include <netinet/in.h>
#include <poll.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fstream>
#include <sstream>
#include <future>
#include <chrono>
#include <cstring>
#include <cerrno>

#include "../../kernel/simulator/TraceManager.h"
#include "../../kernel/TraitsKernel.h"
#include "DistributedExecutionManager.h"
#include "Utils.h"

#define PDEM_MODEL_FILENAME "../../models/pdem_model.gen"

DistributedExecutionManager::DistributedExecutionManager(Simulator* simulator) {
	setSimulator(simulator);
	setModel(this->_simulator->getModels()->current());
    this->originalNumberOfReplications = this->_simulator->getModels()->current()->getSimulation()->getNumberOfReplications();
    Benchmark::getBenchmarkInfo(&_benchmarkInfo);
}

DistributedExecutionManager::~DistributedExecutionManager() {
	this->_model = nullptr;
}

// TODO - Create semaphores for when receiving results from threads or sending the model
// Main methods por DEM - createClientThreadTask and createServerThreadTask
ResultPayload DistributedExecutionManager::createClientThreadTask(SocketData* socketData, std::string filename) {
	
	std::cout << "Starting send\n";
	
	if (!sendCodeMessage(DistributedCommunication::INIT_CONNECTION, socketData->_socket)) {
		std::cout << "[DEM] Error sending handshake with server\n";
	};

	if (!sendSocketData(socketData)) {
		std::cout << "[DEM] Couldn't send socketData\n";
	};

	std::cout << "[DEM] Preparing to receive model\n";

	int fileSize = filename.size();

	if (!sendFileSize(socketData->_socket, fileSize)) {
		std::cout << "[DEM] Couldn't send filesize\n";
	}

	if (!sendFileName(socketData->_socket, filename)) {
		std::cout << "[DEM] Couldn't send filename\n";
	}

	ResultPayload resultPayload;
	std::vector<DataPayload> dataPayloadList;

	if (!receiveResultPayload(socketData, resultPayload, dataPayloadList)) {
		std::cout << "[DEM] Couldn't receive result payload\n";
	}

	std::cout << "Received result payload ----------\n";

	std::cout << "This is the code: " << resultPayload.code << "\n";

	for (const auto& data : dataPayloadList) {
		std::cout << "  Id: " << data.dataId << "\n";
		std::cout << "  Average: " << data.average << std::endl;
		std::cout << "  Variance: " << data.variance << std::endl;
		std::cout << "  Stddeviation: " << data.stddeviation << std::endl;
		std::cout << std::endl;
	}

	return resultPayload;

		// client thread wait on a send of "receiving results"
}

ResultPayload DistributedExecutionManager::createServerThreadTask(SocketData* socketData) {
	ResultPayload resultPayload;
	SocketData* resultSocketData = new SocketData();
	
	if (!receiveCodeMessage(DistributedCommunication::INIT_CONNECTION, socketData->_socket)) {
		resultPayload.code = DistributedCommunication::FAILURE;
		return resultPayload;
	}

	if (!receiveSocketData(resultSocketData, socketData->_socket)) {
		resultPayload.code = DistributedCommunication::FAILURE;
		return resultPayload;
	}
	
	int fileSize;

	if (!receiveFileSize(socketData->_socket, &fileSize)) {
		resultPayload.code = DistributedCommunication::FAILURE;
		return resultPayload;
	}

	std::string filename;

	if (!receiveFileName(socketData->_socket, &filename, fileSize)) {
		resultPayload.code = DistributedCommunication::FAILURE;
		return resultPayload;
	}

	this->_model->getSimulation()->setNumberOfReplications(1);
	
	// Add seed
	//this->_model->getSimulation()->seed
	
	// SEMAPHORE FOR RUNNING THE SIM
	if (!this->_model->getSimulation()->isRunning()) {
		this->_model->getSimulation()->start();
	} else {
		std::cout << "This is running!\n";
	}

	std::cout << "[DEM] Simulation ran\n";

	//this->_model->getSimulation()->get
	//send results


	if (!sendResultPayload(socketData)) {
		std::cout << "[DEM] Failed to send resultPayload, cancelling\n";
		resultPayload.code = DistributedCommunication::FAILURE;
		return resultPayload;
	}

	resultPayload.code = DistributedCommunication::RESULTS;
	return resultPayload;
}

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

bool DistributedExecutionManager::createServerBind(SocketData* socketData) {
	if (bind(socketData->_socket, (struct sockaddr*)&socketData->_address, sizeof(socketData->_address)) == -1) {
		std::string errorString = strerror(errno);
		std::cerr << "Error binding socket: " << errorString << std::endl;

		if (errorString == "Address already in use") {
			return false;
		} else {
			exit(EXIT_FAILURE);
		}
	}

	return true;
}

void DistributedExecutionManager::createServerListen(SocketData* socketData) {
	if (listen(socketData->_socket, 256) == -1) {
		std::cerr << "Error setting up listening socket: " << strerror(errno) << std::endl;
		// Handle the error, possibly by closing the socket and exiting the program
		exit(EXIT_FAILURE);
	}
}

void DistributedExecutionManager::closeConnection(int socketFd) {
	close(socketFd);
}

int DistributedExecutionManager::acceptConnection(SocketData* socketData) {
    socklen_t addressLen = sizeof(socketData->_address);
    std::cout << "Waiting for connection\n";
    int clientSocket = accept(socketData->_socket, (struct sockaddr*)&socketData->_address, &addressLen);

    if (clientSocket == -1) {
        std::cerr << "Error accepting connection: " << strerror(errno) << std::endl;
        // Handle the error, possibly by closing the socket and continuing or exiting the program
    } else {
        std::cout << "Connection accepted\n";
    }

    return clientSocket;
}

SocketData* DistributedExecutionManager::createSocketData(int clientSocketFd) {
	SocketData* socketData = new SocketData();
	socketData->_socket = clientSocketFd;
	return socketData;
}

SocketData* DistributedExecutionManager::createNewSocketDataServer(unsigned int port) {
	SocketData* socketData = new SocketData();
    socketData->_address.sin_family = AF_INET;
    socketData->_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    socketData->_address.sin_port = htons(6000);
    socketData->_socket = createSocket();
	int reuse = 1;
	setsockopt(socketData->_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
	return socketData;
}

bool DistributedExecutionManager::connectToServers() {
    for (auto it = _sockets.begin(); it != _sockets.end();) {
		(*it)->_socket = this->createSocket();

        socklen_t len = sizeof((*it)->_address);
        int result = connect((*it)->_socket, (struct sockaddr*)&(*it)->_address, len);

        if (result != 0) {
            std::cerr << "Error connecting to server at ID: " << (*it)->_id << " - " << strerror(errno) << std::endl;

            // Remove the entry from the socketData list
            it = _sockets.erase(it); // Remove the entry from the list and update the iterator
            continue;
        }

        // If connect was successful, move to the next entry
        ++it;
    }

    return !_sockets.empty(); // Return true if there are still sockets in the list
}


bool DistributedExecutionManager::connectToServerSocket(SocketData* socketData) {
    std::cout << "[EVENT] Attempting to connect to server socket." << std::endl;

    int success = connect(socketData->_socket, (struct sockaddr*)&socketData->_address, sizeof(socketData->_address));

    if (success < 0) {
        std::cout << "[ERROR - FATAL] Failed to connect to the server." << std::endl;
        std::cerr << "Connection failed: " << strerror(errno) << std::endl;
        perror("connectToServerSocket failed");
        return false;
    }

    std::cout << "[EVENT] Connected to the server." << std::endl;
    return true;
}

bool DistributedExecutionManager::receiveCodeMessage(DistributedCommunication expectedCode, int socket) {
    int receivedInt;
    int result = read(socket, &receivedInt, sizeof(int));

    if (result > 0) {
        DistributedCommunication receivedCode = static_cast<DistributedCommunication>(receivedInt);

        std::string msg_code = "[DEM] Received code " + std::to_string(receivedInt) + "\n";
        std::cout << msg_code;

        return (receivedCode == expectedCode);
    } else if (result == 0) {
        // Connection closed by peer
        std::cout << "[DEM] Connection closed by peer\n";
        return false;
    } else {
        perror("[DEM] Error receiving code");
        return false;
    }
}

bool DistributedExecutionManager::sendModel(std::string file, int socket) {
	std::cout << "[DEM] Writing model\n";
	ssize_t bytesSent = write(socket, file.c_str(), file.size());
    if (bytesSent == -1) {
        std::cout << "[DEM] Error writing to socket\n";
		return false;
    }
	std::cout << "[DEM] Model sent\n";
	return true;
}

bool DistributedExecutionManager::receiveModel(std::string* file, int fileSize, int socket) {
    char buffer[fileSize];
    ssize_t bytesRead;

    std::cout << "[DEM] Reading model of size: " << fileSize << "\n";

	bytesRead = read(socket, buffer, fileSize);
    std::string finalFile = std::string(buffer, fileSize);
	file->append(finalFile);

    std::cout << "[DEM] Finished receiving model -----------------\n";
	std::cout << *file << "\n";
	std::cout << "------------------------------------- End of file\n";

    return true;
}

bool DistributedExecutionManager::sendCodeMessage(DistributedCommunication code, int socket) {
	std::cout << "[DEM] sending code message\n";
	int codeInt = static_cast<int>(code);

	int bytesSent = write(socket, &codeInt, sizeof(int));

    if (bytesSent == -1) {
		std::cout << "[DEM] Error sending code message\n";
		perror("Heres why:");
        return false;
    } else if (bytesSent == 0) {
		std::cout << "[DEM] Socket closed\n";
	}

	std::cout << "[DEM] BytesSent: " << bytesSent << "\n";

    std::string msg_code = "[DEM] Sent code " + std::to_string((int)code) + "\n";
	std::cout << msg_code;
	return true;
}

bool DistributedExecutionManager::receiveSocketData(SocketData* socketData, int socket) {
	//SocketData* socketDataTemp;
	int res = read(socket, socketData, sizeof(SocketData));
	if (res > 0) {
		std::cout << "[DEM] Received socketData\n";
		return true;
	}
	if (res < 1) {
		std::cout << "[DEM] Failed to receive socketData\n";
		return false;
	}
	return false;
}

bool DistributedExecutionManager::receiveFileSize(int socket, int* fileSize) {
	ssize_t bytesRead = read(socket, fileSize, sizeof(int));

    if (bytesRead == -1) {
        std::cout << "Error reading from socket: " << strerror(errno) << std::endl;
        return false;
    } else if (bytesRead == 0) {
        std::cout << "Connection closed by the other end" << std::endl;
        return false;
    } else if (bytesRead != sizeof(int)) {
        std::cout << "Unexpected number of bytes read" << std::endl;
        return false;
    }

	std::cout << "[DEM] Finished receiving fileSize\n";

    return true;
}

bool DistributedExecutionManager::sendSocketData(SocketData* socketData) {
	char buffer[sizeof(SocketData)];
	char emptyBuffer[sizeof(SocketData)];

	std::memcpy(buffer, socketData, sizeof(SocketData));
    int bytesSent = write(socketData->_socket, buffer, sizeof(SocketData));

	if (std::memcmp(buffer, emptyBuffer, sizeof(SocketData)) == 0) {
		std::cout << "[DEM] Buffer vazio\n";
		return false;
	}

    if (bytesSent != sizeof(buffer)) {
		std::cout << "[DEM] Couldn't send socket data\n";
		return false;
    } else {
		std::ostringstream oss;
		std::cout << "[DEM] Sent socketData bytes: " << bytesSent << "\n";
		return true;
    }
}

bool DistributedExecutionManager::sendFileSize(int socket, int fileSize) {
	int res = write(socket, &fileSize, sizeof(fileSize));
    if (res == sizeof(fileSize)) {
		std::ostringstream oss;
    	std::cout << "[DEM] fileSize sent: " << fileSize << "\n";
		
		return true;
    } else {
    	std::cout << "[DEM] Failed to send fileSize " << fileSize << "\n";

		return false;
    }
}

bool DistributedExecutionManager::sendResultPayload(SocketData* socketData) {
	const std::string UtilTypeOfStatisticsCollector = Util::TypeOf<StatisticsCollector>();
	const std::string UtilTypeOfCounter = Util::TypeOf<Counter>();
	StatisticsCollector *cstatModel, *cstatSimulation;

	ResultPayload resultPayload;
	std::vector<DataPayload> dataPayloadList;

	List<ModelDataDefinition*>* stats = this->_model->getDataManager()->getDataDefinitionList(Util::TypeOf<StatisticsCollector>());

	for (ModelDataDefinition* data : *stats->list()) {
		cstatModel = dynamic_cast<StatisticsCollector*> (data);
		cstatSimulation = nullptr;

		DataPayload dataPayload;
		dataPayload.dataId = data->getId();
		dataPayload.variance = cstatModel->getStatistics()->variance();
		dataPayload.stddeviation = cstatModel->getStatistics()->stddeviation();
		dataPayload.average = cstatModel->getStatistics()->average();
		dataPayload.min = cstatModel->getStatistics()->min();
		dataPayload.max = cstatModel->getStatistics()->max();

		dataPayloadList.insert(dataPayloadList.end(), dataPayload);
	}

	resultPayload.code = DistributedCommunication::RESULTS;
	resultPayload.dataLength = stats->list()->size();
	resultPayload.threadId = socketData->_id;

	char buffer[sizeof(resultPayload)];
	std::memcpy(buffer, &resultPayload, sizeof(resultPayload));
	int bytesSent = send(socketData->_socket, &resultPayload, sizeof(resultPayload), 0);
	
	if (bytesSent != sizeof(buffer)) {
		std::cout << "[DEM] sendResultPayload error!\n";
		return false;
	}

	this->_model->getTracer()->trace("[DEM] ResultPayload sent");

	for (const DataPayload& dataPayload : dataPayloadList) {
		char dataBuffer[sizeof(DataPayload)];
		std::memcpy(dataBuffer, &dataPayload, sizeof(DataPayload));

		int dataBytesSent = send(socketData->_socket, dataBuffer, sizeof(DataPayload), 0);

		if (dataBytesSent != sizeof(DataPayload)) {
			std::cout << "[DEM] sendDataPayload error!\n";
			return false;
		}
	}

    this->_model->getTracer()->trace("[DEM] DataPayload sent");
	return true;
}

bool DistributedExecutionManager::receiveResultPayload(SocketData* socketData, ResultPayload& resultPayload, std::vector<DataPayload>& dataPayloadList) {
    char buffer[sizeof(ResultPayload)];
    int bytesReceived = read(socketData->_socket, &buffer, sizeof(buffer));

    std::cout << "About to receive results\n";

    if (bytesReceived == sizeof(buffer)) {
        std::cout << "[DEM] receiveResultPayload finished!\n";
        std::memcpy(&resultPayload, buffer, sizeof(ResultPayload));

        // Receive DataPayload items
        for (size_t i = 0; i < resultPayload.dataLength; ++i) {
            char dataBuffer[sizeof(DataPayload)];
            int dataBytesReceived = read(socketData->_socket, &dataBuffer, sizeof(dataBuffer));

            if (dataBytesReceived == sizeof(dataBuffer)) {
                DataPayload dataPayload;
                std::memcpy(&dataPayload, dataBuffer, sizeof(DataPayload));
                dataPayloadList.push_back(dataPayload);
            } else {
                std::cout << "[DEM] receiveDataPayload error!\n";
                return false;
            }
        }

        return true;
    } else {
        std::cout << "[DEM] receiveResultPayload error!\n";
        return false;
    }

	std::cout << "[DEM] receiveResultPayload OK!\n";
	return true;
}

bool DistributedExecutionManager::sendFileName(int socket, std::string filename) {
    int res = write(socket, filename.c_str(), filename.size());
    if (res == static_cast<int>(filename.size())) {
        std::ostringstream oss;
        std::cout << "[DEM] filename sent: " << filename << "\n";
        return true;
    } else {
        std::cout << "[DEM] Failed to send filename " << filename << "\n";
        return false;
    }
}

bool DistributedExecutionManager::receiveFileName(int socket, std::string* filename, int fileSize) {
    char buffer[fileSize];

    ssize_t bytesRead = read(socket, buffer, fileSize);

    if (bytesRead == -1) {
        std::cout << "Error reading from socket: " << strerror(errno) << std::endl;
        return false;
    } else if (bytesRead == 0) {
        std::cout << "Connection closed by the other end" << std::endl;
        return false;
    }

    std::cout << buffer << "\n";
    std::string finalFile(buffer, bytesRead);  // Construct a string from the buffer
    std::cout << finalFile << "\n";
    filename->append(finalFile);

    std::cout << "Finished receiving filename: " << *filename << "\n";

    return true;
}

 void DistributedExecutionManager::writeToFile(const std::string fileName, const std::string& content) {
    // Create an output file stream
    std::ofstream outputFile(fileName);

    // Check if the file is successfully opened
    if (!outputFile.is_open()) {
        std::cerr << "Error opening file: " << fileName << std::endl;
        return;
    }

    // Write the content to the file
    outputFile << content;

    // Close the file stream
    outputFile.close();

    std::cout << "[DEM] File '" << fileName << "' created and written successfully." << std::endl;
	
	std::cout << content << "\n";
	;
}

void DistributedExecutionManager::setModel(Model* model) { _model = model; }

void DistributedExecutionManager::setSimulator(Simulator* simulator) { _simulator = simulator; }

Model* DistributedExecutionManager::getModel() { return _model; }

unsigned int DistributedExecutionManager::getClientPort() { return 6666; }

unsigned int DistributedExecutionManager::getServerPort() { return 6000; }

int DistributedExecutionManager::getNumberThreads() { return _benchmarkInfo._nucleos; }

int DistributedExecutionManager::getRamAmount() { return _benchmarkInfo._memoria; }

std::vector<SocketData*> DistributedExecutionManager::getSocketDataList() { return _sockets; }

void DistributedExecutionManager::appendSocketDataList(SocketData* socketDataItem) {
    if (socketDataItem == nullptr) {
        std::cerr << "Error: Attempting to append nullptr SocketData." << std::endl;
        return;
    }

    _sockets.push_back(socketDataItem);

    // Print the added SocketData
    std::cout << "Added SocketData to the list:" << std::endl;
    std::cout << "SocketData - ID: " << socketDataItem->_id << ", Seed: " << socketDataItem->_seed
              << ", Replication Number: " << socketDataItem->_replicationNumber << std::endl;

    char ipBuffer[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &socketDataItem->_address.sin_addr, ipBuffer, sizeof(ipBuffer));
    std::cout << "    IP Address: " << ipBuffer << ", Port: " << ntohs(socketDataItem->_address.sin_port) << std::endl;

    std::cout << "    Socket: " << socketDataItem->_socket << std::endl;
}


