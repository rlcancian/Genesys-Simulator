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


NetworkIPv4Imp1::NetworkIPv4Imp1(Model* model) {
	_port = 6666; //Default
	_model = model;
}

bool NetworkIPv4Imp1::setPort(int port) {
	if (port > 0 && port < 65535) {
		_port = port;
		return true;
	}
	_model->getTracer()->traceError(Util::TraceLevel::L4_warning, "Fail on setPort. Invalid port number.");
	return false;
}

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

	if (port > 0 && port < 65535) {
		_port = port;
		return true;
	}
	_model->getTracer()->traceError(Util::TraceLevel::L4_warning, "Fail on setPort. Invalid port number.");
	return false;
}

int NetworkIPv4Imp1::getPort() {
	return _port;
}

bool NetworkIPv4Imp1::setIpList() {
	//@TODO NMAP
	return true;
}

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
	}
	for (int i=0; i<_ipList.size(); i++)
        std::cout << _ipList[i] << std::endl;
	return true;
}

bool NetworkIPv4Imp1::setIpList(std::vector<std::string> iplist) {
	_ipList.assign(iplist.begin(), iplist.end());
	for (int i=0; i<_ipList.size(); i++)
        std::cout << _ipList[i] << std::endl;
	return true;
}

void NetworkIPv4Imp1::checkIpList() {
	std::vector<std::string> iplist;
	for (int i=0; i<_ipList.size(); i++) {
		NetworkIPv4Imp1::Socket_Data* temp = newSocketDataClient(i, -1);
		if (clientConnect(temp)) {
			sendIsAliveMessage(temp->_socket);
		}
		delete temp;
	}
}

void NetworkIPv4Imp1::searchLocalNetwork() {
	//@TODO NMAP
}

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

void NetworkIPv4Imp1::sendModel(std::string modelFilePath) {
	//@TODO
}

int NetworkIPv4Imp1::createSocket() {
	int _socket = socket(AF_INET, SOCK_STREAM, 0);
	if (_socket == 0) {
		_model->getTracer()->traceError(Util::TraceLevel::L1_errorFatal, "Fail to create a socket.");
		exit(-1);
	}
	return _socket;
}

NetworkIPv4Imp1::Socket_Data* NetworkIPv4Imp1::newSocketDataClient(int id, int seed) {
	//@TODO
	Socket_Data* data = new Socket_Data();
	data->_id = id;
	data->_seed = seed;
	data->_socket = createSocket();
	data->_address.sin_family = AF_INET;
	data->_address.sin_addr.s_addr = inet_addr(_ipList.at(id).c_str());
	data->_address.sin_port = htons(getPort());
	return data;
}

NetworkIPv4Imp1::Socket_Data* NetworkIPv4Imp1::newSocketDataServer() {
	//@TODO
	Socket_Data* data = new Socket_Data();
	data->_id = -1;
	data->_seed = -1;
	data->_socket = createSocket();
	data->_address.sin_family = AF_INET;
	data->_address.sin_addr.s_addr = htonl(INADDR_ANY);
	data->_address.sin_port = htons(getPort());
	int on = 1;
	setsockopt(data->_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&on,sizeof(on));
	fds[0].fd = data->_socket;
	return data;
}

void NetworkIPv4Imp1::deleteSocketData(Socket_Data* socketData) {
	delete socketData;
}

void NetworkIPv4Imp1::newConnection(int socket) {
	_model->getTracer()->traceError(Util::TraceLevel::L5_event, "Network event. Starting a new connection with a client.");
	fds[1].fd = socket;
	fds[1].events = POLLIN;
	nfds++;
}

void NetworkIPv4Imp1::endConnection() {
	_model->getTracer()->traceError(Util::TraceLevel::L5_event, "Network event. Ending a connection with the client.");
	close(fds[1].fd);
	fds[1].fd = 0;
	nfds--;
}

Util::NetworkCode NetworkIPv4Imp1::getNextNetworkEvent() {
	poll(fds, nfds, -1);
	char buffer[2][1024];

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
						return Util::NetworkCode::C5_Error;
					} else {
						_model->getTracer()->traceError(Util::TraceLevel::L5_event, "Network connection. New client connected.");
						newConnection(newsock);
					}
					fflush(stdout);
					return Util::NetworkCode::C0_Nothing;
				}
				memset(buffer[i], 0, 1024);
				int p = recv(fds[i].fd, buffer[i], 1024, 0);
				if (p > 0) {
					Util::NetworkCode code;
					try {
						code = (Util::NetworkCode)std::stoi(buffer[i]);
					} catch (const std::invalid_argument & e) {
						_model->getTracer()->traceError(Util::TraceLevel::L4_warning, "Network connection error. Received an invalid code.");
						endConnection();
						return Util::NetworkCode::C5_Error;
					} catch (const std::out_of_range & e) {
						_model->getTracer()->traceError(Util::TraceLevel::L4_warning, "Network connection error. Received an invalid code.");
						endConnection();
						return Util::NetworkCode::C5_Error;
					}
					_model->getTracer()->traceError(Util::TraceLevel::L5_event, "Network event. Client send a code.");
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
				return Util::NetworkCode::C5_Error;
            }
		}
	return Util::NetworkCode::C0_Nothing;
}

void NetworkIPv4Imp1::sendIsAliveMessage(int socket = -1) {
	int client_socket;
	if (socket == -1)
		client_socket = fds[1].fd;
	else
		client_socket = socket;
	std::string msg = std::to_string((int)Util::NetworkCode::C1_IsAlive);
	send(client_socket, msg.c_str(), msg.size(), 0);
}

bool NetworkIPv4Imp1::receiveIsAliveMessage() {
	int client_socket;
	if (socket == -1)
		client_socket = fds[1].fd;
	else
		client_socket = socket;
	std::string msg = std::to_string((int)Util::NetworkCode::C1_IsAlive);
	send(client_socket, msg.c_str(), msg.size(), 0);
}

void NetworkIPv4Imp1::sendBenchmarkMessage() {
	Benchmark::Machine_Info info;
	Benchmark::getMachineInfo(&info);
	int client_socket = fds[1].fd;
	char* tmp = reinterpret_cast<char*>(&info);
	send(client_socket, tmp, sizeof(Benchmark::Machine_Info), 0);
}

void NetworkIPv4Imp1::receiveModel(Socket_Data* socketData) {
	int chunk_size = 64;
	char buffer[2][chunk_size];
	memset(buffer[1], 0, chunk_size);

	// std::ofstream file("networkModel.gen");
	std::ofstream file;
	file.open("../../networkModel.gen", std::ios::out | std::ios::binary);
	if (!file.is_open()) {
		_model->getTracer()->traceError(Util::TraceLevel::L1_errorFatal, "Fail to open a file.");
		endConnection();
		return;
	}

	while (1) {
		int p = recv(fds[1].fd, buffer[1], chunk_size, 0);
		if (p == 0)
			break;
		if (p > 0) {
			std::cout << buffer[1] << std::endl;
			file.write(buffer[1], p);
		}
		if (p < 1) {
			std::cout << "error" << std::endl;
		}
	}
	std::cout << buffer[1] << std::endl;
	file.close();
}

bool NetworkIPv4Imp1::clientConnect(Socket_Data* socketData) {
	if ((connect(socketData->_socket, (struct sockaddr *) &socketData->_address, sizeof(socketData->_address))) < 0) {
		_model->getTracer()->traceError(Util::TraceLevel::L1_errorFatal, "Fail to connect to the server.");
		return false;
	}
	_model->getTracer()->traceError(Util::TraceLevel::L5_event, "Connected to the server.");
	return true;
}

void NetworkIPv4Imp1::serverBind(Socket_Data* socketData) {
	bind(socketData->_socket, (struct sockaddr *)&socketData->_address, sizeof(socketData->_address));
}

void NetworkIPv4Imp1::serverListen(Socket_Data* socketData) {
	listen(socketData->_socket, max_nfds);
}

int NetworkIPv4Imp1::receiveSeed(Socket_Data* socketData) {
	int s;
	int rc;
	char incbuff[4096];
	if ((s = accept(socketData->_socket, NULL, NULL)) == -1) {
		std::cout << "Error server Accept" << std::endl;
		exit(1);
	}
	while (1) {
		if (rc = recv(s, incbuff, 4096, 0)<1) {
			if (rc == -1){
				perror("Server: recieving");
				continue;
			}
			printf("client disconnect\n");
			close(s);
			exit(0);
		} else {
			std::cout << std::stoi(incbuff) << std::endl;
			return std::stoi(incbuff);
		}
	}
}

