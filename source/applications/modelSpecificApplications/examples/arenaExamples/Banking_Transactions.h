/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Smart_Dummy.h
 * Author: rlcancian
 *
 * Created on 3 de Setembro de 2019, 18:34
 */

#ifndef BANKING_TRANSACTIONS_H
#define BANKING_TRANSACTIONS_H

#include "../../../BaseGenesysTerminalApplication.h"

class Banking_Transactions : public BaseGenesysTerminalApplication {
public:
	Banking_Transactions();
public:
	virtual int main(int argc, char** argv) override;
};

#endif /* BANKING_TRANSACTIONS_H*/

