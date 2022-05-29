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
#include <fstream>
#include <iostream>
#include <iomanip>
#include <limits>
#include <sstream>

NetworkIPv4Imp1::NetworkIPv4Imp1() {
    std::cout << "ipv4 teste?? " << std::endl;
	_port = 6666; //Default
}

void NetworkIPv4Imp1::setPort(int port) {
	_port = port;
}

void NetworkIPv4Imp1::setIpList(std::string ipList) {
	_ipList = ipList;
}