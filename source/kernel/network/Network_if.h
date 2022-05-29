/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Network_if.cpp
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
	virtual void setPort(int port) = 0;
	virtual void setIpList(std::string ipList) = 0;
};

#endif /* NETWORK_IF_H */

