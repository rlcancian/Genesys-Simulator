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


class Network_if {
public:
	struct Socket_Data {
		int _id;
		int _seed;
		int _socket;
		~Socket_Data() = default;
	};
	virtual void setPort(int port) = 0;
	virtual void setIpList(std::string ipList) = 0;
	virtual int getPort() = 0;
	virtual void sendSocketData(Socket_Data* socketData) = 0;
	virtual void sendModel(std::string modelFilePath) = 0;
	virtual Socket_Data* newSocketData(int id, int seed) = 0;
	virtual void deleteSocketData(Socket_Data* socketData) = 0;
};

#endif /* NETWORK_IF_H */

