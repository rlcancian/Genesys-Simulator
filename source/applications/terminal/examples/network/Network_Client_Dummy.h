/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Network_Client_Dummy.h
 * Author: José Luiz de Souza
 * Email: joseloolo@hotmail.com
 *
 * Created on 28 de Maio de 2022, 23:30
 */

#ifndef NETWORK_CLIENT_DUMMY_H
#define NETWORK_CLIENT_DUMMY_H

#include "../../../BaseGenesysTerminalApplication.h"

class Network_Client_Dummy : public BaseGenesysTerminalApplication {
public:
	Network_Client_Dummy();
public:
	virtual int main(int argc, char** argv);
};

#endif /* NETWORK_CLIENT_DUMMY_H */

