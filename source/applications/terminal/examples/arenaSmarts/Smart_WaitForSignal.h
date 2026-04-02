/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Smart_WaitForSignal.h
 * Author: douglaspereira04
 *
 */

#ifndef SMART_WAITFORSIGNAL_H
#define SMART_WAITFORSIGNAL_H

#include "../../../BaseGenesysTerminalApplication.h"

class Smart_WaitForSignal : public BaseGenesysTerminalApplication {
public:
	Smart_WaitForSignal();
public:
	virtual int main(int argc, char** argv) override;
};
#endif /* SMART_WAITFORSIGNAL_H */
