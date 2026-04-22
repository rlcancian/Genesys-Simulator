/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/class.h to edit this template
 */

/*
 * File:   PickableStationItem.h
 * Author: Prof. Rafael Luiz Cancian, Dr. Eng.
 *
 * Created on 1 de dezembro de 2022, 14:00
 */

#ifndef PICKABLESTATIONITEM_H
#define PICKABLESTATIONITEM_H

#include "kernel/simulator/Model.h"
#include "plugins/data/DiscreteProcessing/Queue.h"
#include "plugins/data/DiscreteProcessing/Resource.h"
#include "../../data/MaterialHandling/Station.h"

class PickableStationItem {
public:
    PickableStationItem(Model* model, std::string stationName, std::string queueName = "", std::string resourceName = "");
    PickableStationItem(Model* model, std::string stationName, std::string expression);
    PickableStationItem(Station* station, Queue* queue);
    PickableStationItem(Station* station, Resource* resource);
    PickableStationItem(Station* station, std::string expression);
    PickableStationItem(Station* station);
    virtual ~PickableStationItem() = default;
public:
    std::string getExpression() const;
    Queue* getQueue() const;
    Resource* getResource() const;
    Station* getStation() const;
    void setExpression(std::string _expression);
    void setQueue(Queue* _queue);
    void setResource(Resource* _resource);
    void setStation(Station* _station);

    std::string getName();
    void _addSimulationControl(SimulationControl* control);
    List<SimulationControl*>* getSimulationControls() const;

    std::string getTypeDC() {return _typeDC;};
private:
private:
    Station* _station = nullptr;
    Resource* _resource = nullptr;
    Queue* _queue = nullptr;
    std::string _expression = "";
    std::string _stationName = "";

    std::string _typeDC = Util::TypeOf<Station>();
    List<SimulationControl*>* _simulationControls = new List<SimulationControl*>();
};

#endif /* PICKABLESTATIONITEM_H */
