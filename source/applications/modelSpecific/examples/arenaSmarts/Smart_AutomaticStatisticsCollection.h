/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Smart_AutomaticStatisticsCollection.h
 * Author: rlcancian
 *
 * Created on 3 de Setembro de 2019, 18:34
 */

#ifndef Smart_AutomaticStatisticsCollection_H
#define Smart_AutomaticStatisticsCollection_H

#include "../../../BaseGenesysTerminalApplication.h"

class Smart_AutomaticStatisticsCollection : public BaseGenesysTerminalApplication {
public:
	Smart_AutomaticStatisticsCollection();
public:
	virtual int main(int argc, char** argv) override;
};

#endif /* Smart_AutomaticStatisticsCollection_H */
