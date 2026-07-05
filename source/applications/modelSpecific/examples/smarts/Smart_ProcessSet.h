/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Smart_ProcessSet.h
 * Author: rlcancian
 *
 * Created on 3 de Setembro de 2019, 18:34
 */

#ifndef SMART_PROCESSSET_H
#define SMART_PROCESSSET_H

#include "../../../BaseGenesysTerminalApplication.h"

class Smart_ProcessSet : public BaseGenesysTerminalApplication {
public:
	Smart_ProcessSet();
public:
	virtual int main(int argc, char** argv) override;
};

#endif /* SMART_PROCESSSET_H */

