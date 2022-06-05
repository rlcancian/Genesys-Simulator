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

NetworkIPv4Imp1::NetworkIPv4Imp1(Model* model) {
    std::cout << "ipv4 teste?? " << std::endl;
	_port = 6666; //Default
	_model = model;
}

void NetworkIPv4Imp1::setPort(int port) {
	_port = port;
}

int NetworkIPv4Imp1::getPort() {
	return _port;
}

void NetworkIPv4Imp1::setIpList(std::string ipList) {
	_ipList = ipList;
}

void NetworkIPv4Imp1::sendSocketData(Socket_Data* socketData) {
	//@TODO
}

void NetworkIPv4Imp1::sendModel(std::string modelFilePath) {
	//@TODO
}

Network_if::Socket_Data* NetworkIPv4Imp1::newSocketData(int id, int seed) {
	//@TODO
	Socket_Data* data = new Socket_Data();
	data->_id = id;
	data->_seed = seed;
	data->_socket = 13;
	return data;
}

void NetworkIPv4Imp1::deleteSocketData(Socket_Data* socketData) {
	delete socketData;
}

