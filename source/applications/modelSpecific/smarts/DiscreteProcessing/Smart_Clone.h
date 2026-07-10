/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Smart_Clone.h
 * Author: rlcancian
 *
 * Created on 30 de Novembro de 2022, 19:25
 */

#ifndef SMART_CLONE_H
#define SMART_CLONE_H

#include "../../../BaseGenesysTerminalApplication.h"

class Smart_Clone : public BaseGenesysTerminalApplication {
public:
	Smart_Clone();
public:
	virtual int main(int argc, char** argv) override;
};

#endif /* SMART_CLONE_H */

