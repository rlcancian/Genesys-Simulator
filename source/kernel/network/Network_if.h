/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Network_if.h
 * Author: José Luiz de Souza
 * Email: joseloolo@hotmail.com
 *
 * Created on 28 de Maio de 2022, 23:30
 */

#ifndef NETWORK_IF_H
#define NETWORK_IF_H

#include <string>
#include <functional>
#include <netinet/in.h>
#include <vector>
#include <fstream>
#include "../util/Util.h"
#include "Benchmark.h"


class Network_if {
public:
	struct Socket_Data {
		int _id;
		int _seed;
		int _numberOfReplications;
		int _socket;
		Benchmark::Machine_Info _machine_info;
		struct sockaddr_in _address;
		~Socket_Data() = default;
	};
	virtual bool setPort(int port) = 0;
	virtual bool setPort(std::string filename) = 0;
	virtual bool setIpList() = 0;
	virtual bool setIpList(std::string filename) = 0;
	virtual bool setIpList(std::vector<std::string> iplist) = 0;
	virtual void checkIpList() = 0;
	virtual void searchLocalNetwork() = 0;
	virtual bool check() = 0;
	virtual int getPort() = 0;
	virtual void sendSocketData(Socket_Data* socketData) = 0;
	virtual Socket_Data* newSocketDataClient(int id) = 0;
	virtual Socket_Data* newSocketDataServer() = 0;
	virtual void deleteSocketData(Socket_Data* socketData) = 0;
	virtual bool clientConnect(Socket_Data* socketData) = 0;
	virtual void serverBind(Socket_Data* socketData) = 0;
	virtual void serverListen(Socket_Data* socketData) = 0;
	virtual Util::NetworkCode getNextNetworkEvent() = 0;

	virtual void sendCodeMessage(Util::NetworkCode code, int socket = -1) = 0;
	virtual bool receiveCodeMessage(Util::NetworkCode code, int socket = -1) = 0;
	virtual void sendBenchmark() = 0;
	virtual void receiveBenchmark(Socket_Data* socketData) = 0;
	virtual void sendModel(Socket_Data* socketData) = 0;
	virtual bool receiveModel(Socket_Data* socketData) = 0;
	virtual void sendSeed(Socket_Data* socketData) = 0;
	virtual bool receiveSeed(Socket_Data* socketData) = 0;
	virtual void sendID(Socket_Data* socketData) = 0;
	virtual bool receiveID(Socket_Data* socketData) = 0;
	virtual void sendNumberOfReplications(Socket_Data* socketData) = 0;
	virtual bool receiveNumberOfReplications(Socket_Data* socketData) = 0;

	virtual void receiveModelResults(Socket_Data* socketData, std::vector<double>* results) = 0;
	virtual void sendModelResults(Socket_Data* socketData) = 0;

	virtual bool isServer() = 0;
	virtual bool isClient() = 0;
	virtual void setServer(bool server) = 0;
	virtual void setClient(bool client) = 0;

	virtual void insertNewData(double value) = 0;

	virtual void opt(int argc, char** argv) = 0;

	virtual void createSockets(std::vector<Socket_Data*>* sockets) = 0;
	virtual void getBenchmarks(std::vector<Socket_Data*>* sockets) = 0;

	// virtual int getValidIpListSize() = 0;
	virtual void reset() = 0;

public:
	std::vector<Socket_Data*> _sockets;
};

#endif /* NETWORK_IF_H */

