#include "NetworkSchedulerStatic.h"
#include "Network_if.h"
#include "../simulator/Model.h"

NetworkSchedulerStatic::NetworkSchedulerStatic(Model* model) {
    _model = model;
}

NetworkSchedulerStatic::~NetworkSchedulerStatic() {
    for (int i = 0; i < _totalOfReplications; i++)
        delete _scheduler.at(i);
}

NetworkScheduler_if::Scheduler_Info* NetworkSchedulerStatic::getNextSimulation(Network_if::Socket_Data* socket) {
    _schedulerMutex.lock();
    if (_remainingReplications == 0) {
        Scheduler_Info* temp;
        time_t time_stamp = -1;
        for (int i = 0; i < _scheduler.size(); i++) {
            if (_scheduler.at(i)->finished)
                continue;

            if (time_stamp == -1) {
                temp = _scheduler.at(i);
                time_stamp = temp->time_stamp;
            } else if (time_stamp > _scheduler.at(i)->time_stamp) {
                temp = _scheduler.at(i);
                time_stamp = temp->time_stamp;
            }
        }

        if (time_stamp == -1) {
            _schedulerMutex.unlock();
            return nullptr;
        } else {
            temp->time_stamp = std::time(NULL);
            // copy(temp, info);
            _schedulerMutex.unlock();
            return temp;
        }
    } else {
        //Create a new info
        Scheduler_Info* temp = new Scheduler_Info();
        temp->id = _idCount;
        temp->seed = _model->getRandom();
        temp->numberOfReplications = 1;
        temp->time_stamp = std::time(NULL);
        temp->finished = false;
        _scheduler.insert(_scheduler.end(), temp);

        _idCount++;
        _remainingReplications--;
        _schedulerMutex.unlock();
        return temp;
        // info = temp;
        // copy(temp, info);
    }
    _schedulerMutex.unlock();
    return nullptr;
}

void NetworkSchedulerStatic::copy(Scheduler_Info* orig, Scheduler_Info* dest) {
    dest->id = orig->id;
    dest->seed = orig->seed;
    dest->numberOfReplications = orig->numberOfReplications;
    dest->time_stamp = orig->time_stamp;
    dest->finished = orig->finished;
}

void NetworkSchedulerStatic::set(int totalOfReplications, std::vector<Network_if::Socket_Data*>* sockets){
    _remainingReplications = totalOfReplications;
    _totalOfReplications = totalOfReplications;
    _idCount = 0;
    _sockets = sockets;
}
