/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ExperimentManagerExampleOfSimulation.h
 * Author: Guilherme
 *
 * Created on 29 de março de 2021, 21:32
 */

#ifndef EXPERIMENTMANAGEREXAMPLEOFSIMULATION_H
#define EXPERIMENTMANAGEREXAMPLEOFSIMULATION_H

class ExperimentManagerExampleOfSimulation : public BaseConsoleGenesysApplication {
public:
    ExperimentManagerExampleOfSimulation();
    ExperimentManagerExampleOfSimulation(const ExperimentManagerExampleOfSimulation& orig);
    virtual ~ExperimentManagerExampleOfSimulation();
    
    virtual int main(int argc, char** argv);
private:

};

#endif /* EXPERIMENTMANAGEREXAMPLEOFSIMULATION_H */

