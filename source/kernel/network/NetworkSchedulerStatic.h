/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   NetworkScheduler.h
 * Author: José Luiz de Souza
 * Email: joseloolo@hotmail.com
 *
 * Created on 28 de Maio de 2022, 23:30
 */

#ifndef NETWORKSCHEDULERSTATIC_H
#define NETWORKSCHEDULERSTATIC_H

#include "NetworkScheduler_if.h"
#include "../simulator/Model.h"
#include <vector>
#include <mutex>
#include "Network_if.h"
#include "../util/Util.h"
#include <ctime>

class NetworkSchedulerStatic : public NetworkScheduler_if {
public:
    NetworkSchedulerStatic(Model* model);
    ~NetworkSchedulerStatic();
public:
    Scheduler_Info* getNextSimulation(Network_if::Socket_Data* socket);
    void set(int totalOfReplications, std::vector<Network_if::Socket_Data*>* sockets);
    void copy(Scheduler_Info* orig, Scheduler_Info* dest);

private:
    Model* _model;
    std::vector<Network_if::Socket_Data*>* _sockets;
    std::vector<Scheduler_Info*> _scheduler;
    std::mutex _schedulerMutex;
    int _idCount;
    int _totalOfReplications;
    int _remainingReplications;

};

#endif