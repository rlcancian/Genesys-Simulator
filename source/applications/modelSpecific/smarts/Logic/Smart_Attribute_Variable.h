/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Smart_Attribute_Variable.h
 * Author: rlcancian
 *
 * Created on 3 de Setembro de 2019, 18:34
 */

#ifndef SMART_ATTRIBUTE_VARIABLE_H
#define SMART_ATTRIBUTE_VARIABLE_H

#include "../../../BaseGenesysTerminalApplication.h"

class Smart_Attribute_Variable : public BaseGenesysTerminalApplication {
public:
	Smart_Attribute_Variable();
public:
	virtual int main(int argc, char** argv) override;
};

#endif /* SMART_ATTRIBUTE_VARIABLE_H */
