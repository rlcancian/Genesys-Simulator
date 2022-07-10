/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   Smart_FSM.h
 * Author: henrique.buss
 *
 * Created on 27 de Maio de 2022, 16:34
 */

#ifndef SMART_FSM_H
#define SMART_FSM_H

#include "../../../BaseGenesysTerminalApplication.h"
#include "../../../../plugins/components/FSM.h"

class Smart_FSM : public BaseGenesysTerminalApplication
{
public:
    Smart_FSM();

public:
    void onEvenToOdd(Model *model, FSM *fsm);
    void onOddToEven(Model *model, FSM *fsm);

public:
    virtual int main(int argc, char **argv);
};

#endif /* SMART_FSM_H */
