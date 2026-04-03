/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Smart_Delay.h
 * Author: rlcancian
 *
 * Created on 3 de Setembro de 2019, 18:34
 */

#ifndef ASSEMBLY_LINE_H
#define ASSEMBLY_LINE_H

#include "../../../BaseGenesysTerminalApplication.h"

class Assembly_Line : public BaseGenesysTerminalApplication {
public:
	Assembly_Line();
public:
	virtual int main(int argc, char** argv) override;
};

#endif /* SMART_DELAY_H */

