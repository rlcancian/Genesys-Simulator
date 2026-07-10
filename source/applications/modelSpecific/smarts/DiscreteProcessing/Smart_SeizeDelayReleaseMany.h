/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Smart_SeizeDelayReleaseMany.h
 * Author: rlcancian
 *
 * Created on 9 de abril de 2021, 18:09
 */

#ifndef MODEL_SEIZEDELAYRELEASEMANY_H
#define MODEL_SEIZEDELAYRELEASEMANY_H

#include "../../../BaseGenesysTerminalApplication.h"

class Smart_SeizeDelayReleaseMany : public BaseGenesysTerminalApplication {
public:
	Smart_SeizeDelayReleaseMany();
public:
	virtual int main(int argc, char** argv) override;
};

#endif /* MODEL_SEIZEDELAYRELEASEMANY_H */

