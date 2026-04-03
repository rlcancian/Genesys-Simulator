/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Smart_DelayBasedOnReplication.h
 * Author: João Janini
 *
 * Created on 3 de Setembro de 2019, 18:34
 */

#ifndef Smart_DELAYBASEDONREPLICATION_H
#define Smart_DELAYBASEDONREPLICATION_H

#include "../../../BaseGenesysTerminalApplication.h"
#include "../../../../plugins/components/Delay.h"


class Smart_DelayBasedOnReplication : public BaseGenesysTerminalApplication {
public:
	Smart_DelayBasedOnReplication();
	void TestEventHandler(SimulationEvent *re);
public:
	virtual int main(int argc, char** argv) override;
	Delay* delay;
	int *proctimes;
};

#endif /* Smart_DelayBasedOnReplication_H */
