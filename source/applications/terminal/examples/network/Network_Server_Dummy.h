/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Network_Server_Dummy.h
 * Author: José Luiz de Souza
 * Email: joseloolo@hotmail.com
 *
 * Created on 28 de Maio de 2022, 23:30
 */

#ifndef NETWORK_SERVER_DUMMY_H
#define NETWORK_SERVER_DUMMY_H

#include "../../../BaseGenesysTerminalApplication.h"

class Network_Server_Dummy : public BaseGenesysTerminalApplication {
public:
	Network_Server_Dummy();
public:
	virtual int main(int argc, char** argv);
};

#endif /* NETWORK_SERVER_DUMMY_H */

