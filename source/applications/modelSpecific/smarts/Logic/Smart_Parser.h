/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Smart_Parser.h
 * Author: rlcancian
 *
 * Created on 5 de Janeiro de 2022, 07:21
 */

#ifndef SMART_PARSER_H
#define SMART_PARSER_H

#include "../../../BaseGenesysTerminalApplication.h"

class Smart_Parser : public BaseGenesysTerminalApplication {
public:
	Smart_Parser();
public:
	virtual int main(int argc, char** argv) override;
};

#endif /* SMART_PARSER_H */

