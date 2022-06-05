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

#include <string>
#include "Network_if.h"

#include "../simulator/Model.h"

class NetworkIPv4Imp1 : public Network_if{
public:
	NetworkIPv4Imp1(Model* model);
	virtual ~NetworkIPv4Imp1() = default;
public:
	// struct Socket_Data {
	// 	int _id;
	// 	int _seed;
	// 	int _socket;
	// 	~Socket_Data() = default;
	// };
public: // inherited from Network_if
	void setPort(int port);
	void setIpList(std::string ipList);
	int getPort();
	void sendSocketData(Socket_Data* socketData);
	void sendModel(std::string modelFilePath);
	Socket_Data* newSocketData(int id, int seed);
	void deleteSocketData(Socket_Data* socketData);
public:
	int getSocket(Socket_Data* socketData);
private:
	int _port;
	std::string _ipList;
	Model* _model;
};
#endif /* NETWORKIPV4IMP1_H */

