/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Smart_SeizeDelayRelease.h
 * Author: rlcancian
 *
 * Created on 24 de Setembro de 2019, 16:43
 */

#ifndef SMART_SEIZEDELAYRELEASE_H
#define SMART_SEIZEDELAYRELEASE_H

#include "../../../BaseGenesysTerminalApplication.h"

class Smart_SeizeDelayRelease : public BaseGenesysTerminalApplication {
public:
	Smart_SeizeDelayRelease();
public:
	virtual int main(int argc, char** argv) override;
};
#endif /* SMART_SEIZEDELAYRELEASE_H */

