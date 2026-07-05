/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Smart_ModuleDisplayVariables.h
 * Author: bepasar
 *
 * Created on 06/12/22, 21:27
 */

#ifndef SMART_MODULEDISPLAYVARIABLES_H
#define SMART_MODULEDISPLAYVARIABLES_H

#include "../../../BaseGenesysTerminalApplication.h"

class Smart_ModuleDisplayVariables : public BaseGenesysTerminalApplication {
public:
	Smart_ModuleDisplayVariables();
public:
	virtual int main(int argc, char** argv) override;
};

#endif /* SMART_MODULEDISPLAYVARIABLES_H */

