/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   buildSimpleModel1.h
 * Author: rafael.luiz.cancian
 *
 * Created on 2 de Outubro de 2018, 19:18
 */

#ifndef COMPLEXMODEL01_H
#define COMPLEXMODEL01_H

#include "../../../BaseGenesysTerminalApplication.h"

class FullSimulationOfComplexModel : public BaseGenesysTerminalApplication {
public:
	FullSimulationOfComplexModel();
public:
	virtual int main(int argc, char** argv) override;
};

#endif /* COMPLEXMODEL01_H */

