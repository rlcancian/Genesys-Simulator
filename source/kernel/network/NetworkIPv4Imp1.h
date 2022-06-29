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
	Socket_Data* newSocketDataClient(int id, int seed, int numberOfReplications);
	Socket_Data* newSocketDataServer();
	void deleteSocketData(Socket_Data* socketData);
	bool clientConnect(Socket_Data* socketData);
	void serverBind(Socket_Data* socketData);
	void serverListen(Socket_Data* socketData);
	void checkIpList();
	void searchLocalNetwork();
	bool check();
	Util::NetworkCode getNextNetworkEvent();


	void sendCodeMessage(Util::NetworkCode code, int socket);
	bool receiveCodeMessage(Util::NetworkCode code, int socket);
	void sendBenchmarkMessage();
	bool receiveModel(Socket_Data* socketData);
	bool receiveSeed(Socket_Data* socketData);
	bool receiveID(Socket_Data* socketData);
	bool receiveNumerOfReplications(Socket_Data* socketData);

	bool isServer();
	bool isClient();
	void setServer(bool server);
	void setClient(bool client);

	void insertNewData(double data);

	void sendModelResults(Socket_Data* socketData);
private:
	int createSocket();
	void newConnection(int socket);
	void endConnection();
	bool createModelResultsFile(std::string filename);
private:
	int _port;
	std::vector<std::string> _ipList;
	Model* _model;
private:
	int max_nfds = 2;
	struct pollfd fds[2] = {{.fd = 0, .events = POLLIN}};
	volatile int nfds = 1;
private:
	bool _isServer = false;
	bool _isClient = false;
	std::vector<double> _data;
};
#endif /* NETWORKIPV4IMP1_H */

