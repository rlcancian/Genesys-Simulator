/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   NetworkIPv4Manager.cpp
 * Author: José Luiz de Souza
 * Email: joseloolo@hotmail.com
 *
 * Created on 28 de Maio de 2022, 23:30
 */

#include "NetworkIPv4Manager.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <limits>
#include <sstream>

NetworkIPv4Manager::NetworkIPv4Manager() {
	// std::cout << "ipv4 teste" << std::endl;
}

void NetworkIPv4Manager::setPort(int port) {
	_port = port;
}

void NetworkIPv4Manager::setIpList(std::string ipList) {
	_ipList = ipList;
}