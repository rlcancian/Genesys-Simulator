/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Smart_DefiningControlLogic.h
 * Author: rlcancian
 *
 * Created on 3 de Setembro de 2019, 18:34
 */

#ifndef Smart_DEFININGCONTROLLOGIC_H
#define Smart_DEFININGCONTROLLOGIC_H

#include "../../../BaseGenesysTerminalApplication.h"
#include "plugins/components/DiscreteProcessing/Delay.h"


class Smart_DefiningControlLogic : public BaseGenesysTerminalApplication {
public:
	Smart_DefiningControlLogic();
public:
	virtual int main(int argc, char** argv) override;
};

#endif /* Smart_DEFININGCONTROLLOGIC_H */
