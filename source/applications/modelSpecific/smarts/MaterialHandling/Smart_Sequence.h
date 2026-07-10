/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Smart_Sequence.h
 * Author: rlcancian
 *
 * Created on 12 de março de 2021, 15:39
 */

#ifndef SMART_SEQUENCE_H
#define SMART_SEQUENCE_H

#include "../../../BaseGenesysTerminalApplication.h"

class Smart_Sequence : public BaseGenesysTerminalApplication {
public:
	Smart_Sequence();
public:
	virtual int main(int argc, char** argv) override;
};

#endif /* SMART_SEQUENCE_H */

