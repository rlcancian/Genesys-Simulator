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

class Network_if {
public:
	struct Socket_Data {
		int _id;
		int _seed;
		int _socket;
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
	virtual void sendModel(std::string modelFilePath) = 0;
	virtual Socket_Data* newSocketDataClient(int id, int seed) = 0;
	virtual Socket_Data* newSocketDataServer() = 0;
	virtual void deleteSocketData(Socket_Data* socketData) = 0;
	virtual bool clientConnect(Socket_Data* socketData) = 0;
	virtual void serverBind(Socket_Data* socketData) = 0;
	virtual void serverListen(Socket_Data* socketData) = 0;
	virtual int receiveSeed(Socket_Data* socketData) = 0;
	virtual Util::NetworkCode getNextNetworkEvent() = 0;

	virtual void sendIsAliveMessage(int socket) = 0;
	virtual bool receiveIsAliveMessage() = 0;
	virtual void sendBenchmarkMessage() = 0;
	virtual void receiveModel(Socket_Data* socketData) = 0;
};

#endif /* NETWORK_IF_H */

