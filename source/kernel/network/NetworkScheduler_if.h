/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   NetworkScheduler_if.h
 * Author: José Luiz de Souza
 * Email: joseloolo@hotmail.com
 *
 * Created on 28 de Maio de 2022, 23:30
 */

#ifndef NETWORKSCHEDULER_IF_H
#define NETWORKSCHEDULER_IF_H

#include "NetworkScheduler_if.h"
#include <vector>
#include <mutex>
#include <ctime>
#include "Network_if.h"
#include "../util/Util.h"

class NetworkScheduler_if {
    public:
    struct Scheduler_Info {
		int id;
        int seed;
        time_t time_stamp;
        bool finished;
        int numberOfReplications;
        ~Scheduler_Info() = default;
	};
public:
    virtual Scheduler_Info* getNextSimulation(Network_if::Socket_Data* socket) = 0;
    virtual void set(int totalOfReplications, std::vector<Network_if::Socket_Data*>* sockets) = 0;
    virtual void copy(Scheduler_Info* orig, Scheduler_Info* dest) = 0;
};

#endif