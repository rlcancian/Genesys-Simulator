/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   NetworkIPv4Manager.h
 * Author: José Luiz de Souza
 * Email: joseloolo@hotmail.com
 *
 * Created on 28 de Maio de 2022, 23:30
 */

#ifndef NETWORKIPV4MANAGER_H
#define NETWORKIPV4MANAGER_H

#include <string>

class NetworkIPv4Manager{
public:
	NetworkIPv4Manager();
	virtual ~NetworkIPv4Manager() = default;
public: // inherited from Network_if
	virtual void setPort(int port);
	virtual void setIpList(std::string ipList);
private:
	int _port;
	std::string _ipList;
};

#endif /* NETWORKIPV4MANAGER_H */

