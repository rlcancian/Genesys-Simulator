/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   NetworkIPv4Imp1.h
 * Author: José Luiz de Souza
 * Email: joseloolo@hotmail.com
 *
 * Created on 28 de Maio de 2022, 23:30
 */

#ifndef NETWORKIPV4IMP1_H
#define NETWORKIPV4IMP1_H

#include "Network_if.h"
#include <string>
#include <poll.h>

#include "../simulator/Model.h"

class NetworkIPv4Imp1 : public Network_if{
public:
	NetworkIPv4Imp1(Model* model);
	virtual ~NetworkIPv4Imp1() = default;
public: // inherited from Network_if
	bool setPort(int port);
	bool setPort(std::string filename);
	bool setIpList();
	bool setIpList(std::string filename);
	bool setIpList(std::vector<std::string> iplist);
	int getPort();
	void sendSocketData(Socket_Data* socketData);
	void sendModel(std::string modelFilePath);
	Socket_Data* newSocketDataClient(int id, int seed);
	Socket_Data* newSocketDataServer();
	void deleteSocketData(Socket_Data* socketData);
	bool clientConnect(Socket_Data* socketData);
	void serverBind(Socket_Data* socketData);
	void serverListen(Socket_Data* socketData);
	void checkIpList();
	void searchLocalNetwork();
	bool check();
	Util::NetworkCode getNextNetworkEvent();

	int receiveSeed(Socket_Data* socketData);

	void sendIsAliveMessage(int socket);
	bool receiveIsAliveMessage();
	void sendBenchmarkMessage();
	void receiveModel(Socket_Data* socketData);
private:
	int createSocket();
	void newConnection(int socket);
	void endConnection();
private:
	int _port;
	std::vector<std::string> _ipList;
	Model* _model;
private:
	int max_nfds = 2;
	struct pollfd fds[2] = {{.fd = 0, .events = POLLIN}};
	volatile int nfds = 1;
};
#endif /* NETWORKIPV4IMP1_H */

