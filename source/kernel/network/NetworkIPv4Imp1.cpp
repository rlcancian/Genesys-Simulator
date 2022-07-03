/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   NetworkIPv4Imp1.cpp
 * Author: José Luiz de Souza
 * Email: joseloolo@hotmail.com
 *
 * Created on 28 de Maio de 2022, 23:30
 */

#include "NetworkIPv4Imp1.h"
#include <string>
#include <iostream>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <limits>
#include <sstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
// #include <unistd.h>
#include <arpa/inet.h>
// #include <stdlib.h>

#include "Benchmark.h"

//Client & Server
NetworkIPv4Imp1::NetworkIPv4Imp1(Model* model) {
	_port = 6666; //Default
	_model = model;
}

void NetworkIPv4Imp1::opt(int argc, char** argv) {
	std::vector<std::string> iplist;
	int opt;
    while((opt = getopt(argc, argv, ":p:f:i:h")) != -1)
    {
        if(opt == 'p'){
			int port;
			try {
			port = std::stoi(optarg);
			} catch (const std::invalid_argument & e) {
				_model->getTracer()->traceError(Util::TraceLevel::L1_errorFatal, "Failed on getopt. Invalid parameter.");
				break;
			}
            _model->getSimulation()->getNetwork()->setPort(port);
        }else if(opt == 'f'){
			_model->getSimulation()->getNetwork()->setIpList(optarg);
		}else if(opt == 'i'){
			iplist.insert(iplist.end(), optarg);
        }else if(opt == 'h'){
			std::string msg;
			msg = "Usage: genesys [options...]\n";
			msg += "Options:\n";
			msg += "   -p PORT         define que listening port\n";
			msg += "   -f FILE         define a file to read a ip list\n";
			msg += "   -i IP           define a ip\n";
            _model->getTracer()->traceError(Util::TraceLevel::L1_errorFatal, msg);
			exit(0);
        }
    }
	_model->getSimulation()->getNetwork()->setIpList(iplist);
	return;
}

bool NetworkIPv4Imp1::isServer() {
	return _isServer;
}

bool NetworkIPv4Imp1::isClient() {
	return _isClient;
}

void NetworkIPv4Imp1::setServer(bool server) {
	_isServer = server;
}

void NetworkIPv4Imp1::setClient(bool client) {
	_isClient = client;
}

void NetworkIPv4Imp1::insertNewData(double value) {
	_data.insert(_data.end(), value);
}

void NetworkIPv4Imp1::createSockets(std::vector<Socket_Data*>* sockets) {
	int j = 0;
	for (int i = 0; i < _ipList.size(); i++) {
		// int r = _model->getRandom();
		std::string msg;
		NetworkIPv4Imp1::Socket_Data* temp = newSocketDataClient(j);
		if (clientConnect(temp)) {
			sendCodeMessage(Util::NetworkCode::C1_IsAlive, temp->_socket);
			if (receiveCodeMessage(Util::NetworkCode::C1_IsAlive, temp->_socket)) {
				sockets->insert(sockets->end(), temp);
				j++;
				msg = "[OK  ] - " + _ipList.at(i);
				_model->getTracer()->traceError(Util::TraceLevel::L5_event, msg);
			} else {
				deleteSocketData(temp);
				msg = "[FAIL] - " + _ipList.at(i);
				_model->getTracer()->traceError(Util::TraceLevel::L5_event, msg);
			}
		} else {
			deleteSocketData(temp);
			msg = "[FAIL] - " + _ipList.at(i);
			_model->getTracer()->traceError(Util::TraceLevel::L5_event, msg);
		}
	}
}

void NetworkIPv4Imp1::getBenchmarks(std::vector<Socket_Data*>* sockets) {
	for (int i = 0; i < sockets->size(); i++) {
		sendCodeMessage(Util::NetworkCode::C2_Benchmark, sockets->at(i)->_socket);
		receiveBenchmark(sockets->at(i));
	}
	_model->getTracer()->traceError(Util::TraceLevel::L5_event, "getBenchmarks OK!");
}

void NetworkIPv4Imp1::sendModelResults(Socket_Data* socketData) {
	std::string filename = "modelresults";
	createModelResultsFile(filename);
	std::fstream file;
	file.open(filename, std::ios::in | std::ios::binary);
	std::string contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	send(fds[1].fd , contents.c_str() , contents.length() , 0);
	file.close();
}

void NetworkIPv4Imp1::receiveModelResults(Socket_Data* socketData, std::vector<double>* results) {
	sendCodeMessage(Util::NetworkCode::C4_Results, socketData->_socket);
	//If we don't receive a C4_Results, then the operation was canceled
	if (!receiveCodeMessage(Util::NetworkCode::C4_Results, socketData->_socket))
		return;
	char buffer[512];
	memset(buffer, 0, 512);
	std::ofstream file;
	file.open("results.txt", std::ios::out | std::ios::binary);
	while (1) {
		int p = recv(socketData->_socket, buffer, 512, 0);
		if (p == 0) {
			break;
		}
		if (p > 0) {
			file.write(buffer, p);
			if (p < 64)
				break;
		}
		if (p < 1) {
			_model->getTracer()->traceError(Util::TraceLevel::L1_errorFatal, "Fail to receive the model results.");
		}
	}
	file.close();

	std::ifstream file2;
	file2.open("results.txt", std::ios::in | std::ios::binary);
	while (file2.good() && !file2.eof()) {
		double temp = 0;
		file2.read((char*)&temp, sizeof(double));
		results->insert(results->end(), temp);
		std::cout << temp << std::endl;
	}
	file2.close();
}

bool NetworkIPv4Imp1::createModelResultsFile(std::string filename) {
	std::ofstream file;
	file.open(filename, std::ios::out | std::ios::binary);
	if (!file.is_open()) {
		_model->getTracer()->traceError(Util::TraceLevel::L1_errorFatal, "Fail to open a file to salve the mddel results.");
		return false;
	}

	for (int i = 0; i < _data.size(); i++) {
		double temp = _data.at(i);
		std::cout << temp << std::endl;
		file.write(reinterpret_cast<const char*>(&temp), sizeof(temp));
	}

	file.close();
	return true;
}

void NetworkIPv4Imp1::reset() {
	_data.clear();
}

//Client
bool NetworkIPv4Imp1::setPort(int port) {
	if (port > 0 && port < 65535) {
		_port = port;
		std::string msg = "New port defined: " + std::to_string(port);
		_model->getTracer()->traceError(Util::TraceLevel::L5_event, msg);
		return true;
	}
	_model->getTracer()->traceError(Util::TraceLevel::L4_warning, "Fail on setPort. Invalid port number.");
	return false;
}

//Client
bool NetworkIPv4Imp1::setPort(std::string filename) {
	std::fstream file;
    file.open(filename, std::ios::in);
	if (!file.is_open()) {
		_model->getTracer()->traceError(Util::TraceLevel::L1_errorFatal, "Failed on setPort. Fail to open a file.");
		return false;
	}
	std::string temp;
	int port;
	file >> temp;
	try {
		port = std::stoi(temp);
	} catch (const std::invalid_argument & e) {
		_model->getTracer()->traceError(Util::TraceLevel::L4_warning, "Failed on setPort. Invalid argument in file.");
		return false;
	}
	file.close();
	return setPort(port);
}

//Client
int NetworkIPv4Imp1::getPort() {
	return _port;
}

//Client
bool NetworkIPv4Imp1::setIpList() {
	//@TODO NMAP
	return true;
}

//Client
bool NetworkIPv4Imp1::setIpList(std::string filename) {
	std::fstream file;
    file.open(filename, std::ios::in);
	if (!file.is_open()) {
		_model->getTracer()->traceError(Util::TraceLevel::L1_errorFatal, "Failed on setIpList. Fail to open a file.");
		return false;
	}
	std::string temp;
	while (file >> temp)
	{
		_ipList.insert(_ipList.end(), temp);
		std::string msg = "New ip inserted: " + temp;
		_model->getTracer()->traceError(Util::TraceLevel::L5_event, msg);
	}
	file.close();
	return true;
}

//Client
bool NetworkIPv4Imp1::setIpList(std::vector<std::string> iplist) {
	for (int i=0; i<iplist.size(); i++) {
		_ipList.insert(_ipList.end(), iplist.at(i));
        std::string msg = "New ip inserted: " + iplist.at(i);
		_model->getTracer()->traceError(Util::TraceLevel::L5_event, msg);
	}
	return true;
}

//Client
void NetworkIPv4Imp1::checkIpList() {
	std::vector<std::string> iplist;
	for (int i=0; i<_ipList.size(); i++) {
		std::string msg = "";
		NetworkIPv4Imp1::Socket_Data* temp = newSocketDataClient(i);
		if (clientConnect(temp)) {
			sendCodeMessage(Util::NetworkCode::C1_IsAlive, temp->_socket);
			if (receiveCodeMessage(Util::NetworkCode::C1_IsAlive, temp->_socket)) {
				iplist.insert(iplist.end(), _ipList.at(i));
			} else {
				msg = "[FAIL] - " + _ipList.at(i);
				_model->getTracer()->traceError(Util::TraceLevel::L5_event, msg);
			}
		} else {
			msg = "[FAIL] - " + _ipList.at(i);
			_model->getTracer()->traceError(Util::TraceLevel::L5_event, msg);
		}
		deleteSocketData(temp);
	}
	_ipList.clear();
	setIpList(iplist);
	_model->getTracer()->traceError(Util::TraceLevel::L5_event, "CheckIpList finished.");
}

//Client
void NetworkIPv4Imp1::searchLocalNetwork() {
	//@TODO NMAP
}

//Client
bool NetworkIPv4Imp1::check() {
	checkIpList();
	if (_ipList.size() == 0) {
		return false;
	}
	return true;
}

void NetworkIPv4Imp1::sendSocketData(Socket_Data* socketData) {
	//@TODO
}

//Client & Server
int NetworkIPv4Imp1::createSocket() {
	int _socket = socket(AF_INET, SOCK_STREAM, 0);
	if (_socket == 0) {
		_model->getTracer()->traceError(Util::TraceLevel::L1_errorFatal, "Fail to create a socket.");
		exit(-1);
	}
	return _socket;
}

//Client
NetworkIPv4Imp1::Socket_Data* NetworkIPv4Imp1::newSocketDataClient(int id) {
	Socket_Data* data = new Socket_Data();
	data->_id = -1;
	data->_seed = -1;
	data->_numberOfReplications = -1;
	data->_socket = createSocket();
	data->_address.sin_family = AF_INET;
	data->_address.sin_addr.s_addr = inet_addr(_ipList.at(id).c_str());
	data->_address.sin_port = htons(getPort());
	return data;
}

//Server
NetworkIPv4Imp1::Socket_Data* NetworkIPv4Imp1::newSocketDataServer() {
	Socket_Data* data = new Socket_Data();
	data->_id = -1;
	data->_seed = -1;
	data->_numberOfReplications = -1;
	data->_socket = createSocket();
	data->_address.sin_family = AF_INET;
	data->_address.sin_addr.s_addr = htonl(INADDR_ANY);
	data->_address.sin_port = htons(getPort());
	int on = 1;
	setsockopt(data->_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&on,sizeof(on));
	fds[0].fd = data->_socket;
	return data;
}

//Client & Server
void NetworkIPv4Imp1::deleteSocketData(Socket_Data* socketData) {
	close(socketData->_socket);
	delete socketData;
}

//Servers
void NetworkIPv4Imp1::newConnection(int socket) {
	_model->getTracer()->traceError(Util::TraceLevel::L5_event, "Network event. Starting a new connection with a client.");
	fds[1].fd = socket;
	fds[1].events = POLLIN;
	nfds++;
}

//Server
void NetworkIPv4Imp1::endConnection() {
	_model->getTracer()->traceError(Util::TraceLevel::L5_event, "Network event. Ending a connection with the client.");
	close(fds[1].fd);
	fds[1].fd = 0;
	nfds--;
}

//Server
Util::NetworkCode NetworkIPv4Imp1::getNextNetworkEvent() {
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
						_model->getTracer()->traceError(Util::TraceLevel::L4_warning, "Network error. Client limit reached.");
						close(newsock);
						return Util::NetworkCode::C6_Error;
					} else {
						_model->getTracer()->traceError(Util::TraceLevel::L5_event, "Network connection. New client connected.");
						newConnection(newsock);
					}
					fflush(stdout);
					return Util::NetworkCode::C0_Nothing;
				}
				int p = recv(fds[i].fd, &buffer, sizeof(int), 0);
				if (p > 0) {
					Util::NetworkCode code;
					code = (Util::NetworkCode)ntohl(buffer);
					_model->getTracer()->traceError(Util::TraceLevel::L5_event, "Network event. Client send a code.");
					std::cout << (int)code << std::endl;
					fflush(stdout);
					return code;
				} else {
					endConnection();
					Util::NetworkCode::C0_Nothing;
				}
				break;
			}
			case POLLNVAL:
			case POLLPRI:
			case POLLOUT:
			case POLLERR:
			case POLLHUP:
			default:
				_model->getTracer()->traceError(Util::TraceLevel::L4_warning, "Network error. Unespected revents.");
				endConnection();
				return Util::NetworkCode::C6_Error;
            }
		}
	return Util::NetworkCode::C0_Nothing;
}

//Client & Server
void NetworkIPv4Imp1::sendCodeMessage(Util::NetworkCode code, int socket = -1) {
	int client_socket = (socket == -1) ? fds[1].fd : socket;
	int msg = htonl((int)code);
	send(client_socket, &msg, sizeof(int), 0);
	std::string msg_code = "Sending code " + std::to_string((int)code) + "...";
	_model->getTracer()->traceError(Util::TraceLevel::L5_event, msg_code);
}

//Cliente & Server
bool NetworkIPv4Imp1::receiveCodeMessage(Util::NetworkCode code, int socket = -1) {
	int client_socket = (socket == -1) ? fds[1].fd : socket;
	// struct timeval tv;
	// tv.tv_sec = 3;
	// tv.tv_usec = 0;
	// setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
	int buffer = -1;
	int p = recv(client_socket, &buffer, sizeof(int), 0);
	if (p > 0) {
		Util::NetworkCode _code;
		_code = (Util::NetworkCode)ntohl(buffer);
		std::string msg_code = "Received code " + std::to_string((int)_code) + "...";
		_model->getTracer()->traceError(Util::TraceLevel::L5_event, msg_code);
		return (_code == code);
	} else {
		return false;
	}
	return false;
}

//Server
void NetworkIPv4Imp1::sendBenchmark() {
	Benchmark::Machine_Info info;
	Benchmark::getMachineInfo(&info);
	int client_socket = fds[1].fd;
	char* tmp = reinterpret_cast<char*>(&info);
	send(client_socket, tmp, sizeof(Benchmark::Machine_Info), 0);
}

//Client
void NetworkIPv4Imp1::receiveBenchmark(Socket_Data* socketData) {
	int p = recv(socketData->_socket, &socketData->_machine_info, sizeof(Benchmark::Machine_Info), 0);
	if (p < 1) {
		_model->getTracer()->traceError(Util::TraceLevel::L1_errorFatal, "Error receiving the benchmark.");
	}
}

//Client
void NetworkIPv4Imp1::sendNumberOfReplications(Socket_Data* socketData) {
	int a = htonl(socketData->_numberOfReplications);
	char* tmp = reinterpret_cast<char*>(&a);
	send(socketData->_socket, tmp, sizeof(int), 0);
}

//Server
bool NetworkIPv4Imp1::receiveNumberOfReplications(Socket_Data* socketData) {
	int buffer = -1;
	int p = recv(fds[1].fd, &buffer, sizeof(int), 0);
	if (p > 0) {
		int numberOfReplications = ntohl(buffer);
		_model->getTracer()->traceError(Util::TraceLevel::L5_event, "NumberOfReplications received.");
		socketData->_numberOfReplications = numberOfReplications;
		return true;
	}
	if (p < 1) {
		_model->getTracer()->traceError(Util::TraceLevel::L1_errorFatal, "Fail to receive the seed.");
		return false;
	}
	return false;
}

//Client
void NetworkIPv4Imp1::sendSeed(Socket_Data* socketData) {
	int a = htonl(socketData->_seed);
	char* tmp = reinterpret_cast<char*>(&a);
	send(socketData->_socket, tmp, sizeof(int), 0);
}

//Server
bool NetworkIPv4Imp1::receiveSeed(Socket_Data* socketData) {
	int buffer = -1;
	int p = recv(fds[1].fd, &buffer, sizeof(int), 0);
	if (p > 0) {
		int seed = ntohl(buffer);
		_model->getTracer()->traceError(Util::TraceLevel::L5_event, "Seed received.");
		socketData->_seed = seed;
		return true;
	}
	if (p < 1) {
		_model->getTracer()->traceError(Util::TraceLevel::L1_errorFatal, "Fail to receive the seed.");
		return false;
	}
	return false;
}

//Client
void NetworkIPv4Imp1::sendID(Socket_Data* socketData) {
	int a = htonl(socketData->_id);
	char* tmp = reinterpret_cast<char*>(&a);
	send(socketData->_socket, tmp, sizeof(int), 0);
}

//Server
bool NetworkIPv4Imp1::receiveID(Socket_Data* socketData) {
	int buffer = -1;
	int p = recv(fds[1].fd, &buffer, sizeof(int), 0);
	if (p > 0) {
		int id = ntohl(buffer);
		_model->getTracer()->traceError(Util::TraceLevel::L5_event, "ID received.");
		socketData->_id = id;
		return true;
	}
	if (p < 1) {
		_model->getTracer()->traceError(Util::TraceLevel::L1_errorFatal, "Fail to receive the id.");
		return false;
	}
	return false;
}

void NetworkIPv4Imp1::sendModel(Socket_Data* socketData) {
	sendCodeMessage(Util::NetworkCode::C3_Model, socketData->_socket);
	sendID(socketData);
	sendSeed(socketData);
	sendNumberOfReplications(socketData);
	std::fstream file;
    file.open("modelo.gen", std::ios::in | std::ios::binary);
    std::string contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	send(socketData->_socket , contents.c_str() , contents.length() , 0 );
	if (receiveCodeMessage(Util::NetworkCode::C3_Model, socketData->_socket)) {
		_model->getTracer()->traceError(Util::TraceLevel::L5_event, "sendModel OK!");
	} else {
		_model->getTracer()->traceError(Util::TraceLevel::L5_event, "sendModel ERROR!");
	}
}

//Server
bool NetworkIPv4Imp1::receiveModel(Socket_Data* socketData) {
	if (!receiveID(socketData)) {
		endConnection();
		return false;
	}
	if (!receiveSeed(socketData)) {
		endConnection();
		return false;
	}
	if (!receiveNumberOfReplications(socketData)) {
		endConnection();
		return false;
	}
	std::cout << socketData->_id << std::endl;
	std::cout << socketData->_seed << std::endl;
	std::cout << socketData->_numberOfReplications << std::endl;
	int chunk_size = 512;
	char buffer[2][chunk_size];
	memset(buffer[1], 0, chunk_size);

	std::ofstream file;
	file.open("networkModel.gen", std::ios::out | std::ios::binary);
	if (!file.is_open()) {
		_model->getTracer()->traceError(Util::TraceLevel::L1_errorFatal, "Fail to open a file.");
		endConnection();
		return false;
	}

	while (1) {
		int p = recv(fds[1].fd, buffer[1], chunk_size, 0);
		std::cout << p << std::endl;
		if (p == 0) {
			break;
		}
		if (p > 0) {
			file.write(buffer[1], p);
			if (p < chunk_size)
				break;
		}
		if (p < 1) {
			_model->getTracer()->traceError(Util::TraceLevel::L1_errorFatal, "Fail to receive the model.");
			endConnection();
			return false;
		}
	}
	_model->getTracer()->traceError(Util::TraceLevel::L5_event, "Model received.");
	file.close();
	return true;
}

//Client
bool NetworkIPv4Imp1::clientConnect(Socket_Data* socketData) {
	if ((connect(socketData->_socket, (struct sockaddr *) &socketData->_address, sizeof(socketData->_address))) < 0) {
		_model->getTracer()->traceError(Util::TraceLevel::L1_errorFatal, "Fail to connect to the server.");
		return false;
	}
	_model->getTracer()->traceError(Util::TraceLevel::L5_event, "Connected to the server.");
	return true;
}

//Server
void NetworkIPv4Imp1::serverBind(Socket_Data* socketData) {
	bind(socketData->_socket, (struct sockaddr *)&socketData->_address, sizeof(socketData->_address));
}

//Server
void NetworkIPv4Imp1::serverListen(Socket_Data* socketData) {
	listen(socketData->_socket, max_nfds);
}

// int NetworkIPv4Imp1::getValidIpListSize() {
// 	return _ipList.size();
// }