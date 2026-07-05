/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Smart_ParallelProcessingOfEntities.h
 * Author: Luiz e Guilherme
 *
 * Created on Dezembro de 2022
 */

#ifndef SMART_PARALLEL_PROCESSING_OF_ENTITIES_H
#define SMART_PARALLEL_PROCESSING_OF_ENTITIES_H

#include "../../../BaseGenesysTerminalApplication.h"

class Smart_ParallelProcessingOfEntities : public BaseGenesysTerminalApplication {
public:
    Smart_ParallelProcessingOfEntities();
public:
    virtual int main(int argc, char** argv) override;
};

#endif /* SMART_PARALLEL_PROCESSING_OF_ENTITIES_H */
