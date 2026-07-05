/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Smart_WaitSignal.h
 * Author: rlcancian
 *
 * Created on 3 de Setembro de 2019, 18:34
 */

#ifndef SMART_SELECTINGROUTEBASEDONPROBABILITY_H
#define SMART_SELECTINGROUTEBASEDONPROBABILITY_H

#include "../../../BaseGenesysTerminalApplication.h"

class Smart_SelectingRouteBasedOnProbability : public BaseGenesysTerminalApplication {
public:
	Smart_SelectingRouteBasedOnProbability();
public:
	virtual int main(int argc, char** argv) override;
};

#endif /* SMART_SELECTINGROUTEBASEDONPROBABILITY_H */

