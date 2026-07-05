/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Smart_Dummy.h
 * Author: rlcancian
 *
 * Created on 3 de Setembro de 2019, 18:34
 */

#ifndef SMART_RESOURCECOSTING_H
#define SMART_RESOURCECOSTING_H

#include "../../../BaseGenesysTerminalApplication.h"

class Smart_ResourceCosting : public BaseGenesysTerminalApplication {
public:
	Smart_ResourceCosting();
public:
	virtual int main(int argc, char** argv) override;
};

#endif /* SMART_RESOURCECOSTING_H */

