/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   Smart_WaitSignal.cpp
 * Author: rlcancian
 * Author: douglaspereira04
 *
 * Created on 3 de Setembro de 2019, 18:34
 */

#include "Smart_SelectingRouteBasedOnProbability.h"

// you have to included need libs

// GEnSyS Simulator
#include "../../../../kernel/simulator/Simulator.h"
#include "../../../../kernel/util/Util.h"

// Model Components
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/Route.h"
#include "../../../../plugins/components/Enter.h"
#include "../../../../plugins/components/Dispose.h"
#include "../../../../plugins/components/Decide.h"
#include "../../../../plugins/components/Process.h"

#include "../../../../plugins/components/QueueableItem.h"
#include "../../../../plugins/components/SeizableItem.h"

#include "../../../../plugins/data/Resource.h"
#include "../../../../plugins/data/Queue.h"
#include "../../../TraitsApp.h"

// Model data definitions

Smart_SelectingRouteBasedOnProbability::Smart_SelectingRouteBasedOnProbability() {
}

/**
 * This is the main function of the application.
 * It instanciates the simulator, builds a simulation model and then simulate that model.
 */
int Smart_SelectingRouteBasedOnProbability::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();
	// create model

        //arrival
	Create* part_arrive = plugins->newInstance<Create>(model, "PartArrive");
	part_arrive->setEntityTypeName("Entity1");
	part_arrive->setTimeBetweenCreationsExpression("expo(5.0)", Util::TimeUnit::minute);
	part_arrive->setFirstCreation(0.0);
        part_arrive->setReportStatistics(true);
        
        Enter* part_arrival = plugins->newInstance<Enter>(model, "PartArrival");
        part_arrival->setStationName("ArrivalStation");
        part_arrival->setReportStatistics(true);
        
        Route* route_to_decision = plugins->newInstance<Route>(model, "RouteToDecisionArea");
        route_to_decision->setRouteTimeExpression("5");
        route_to_decision->setRouteTimeTimeUnit(Util::TimeUnit::minute);
        route_to_decision->setStationName("Decision");
        route_to_decision->setReportStatistics(true);
        
        //decision
        Enter* decision_area = plugins->newInstance<Enter>(model, "DecisionArea");
        decision_area->setStationName("Decision");
        decision_area->setReportStatistics(true);
        
        Decide* decide = plugins->newInstance<Decide>(model, "WhereShouldThePartsBeProcessed");
        decide->getConditions()->insert("unif(0.0,1.0)<=0.66");
        decide->setReportStatistics(true);
        
        Route* route_to_machine_1 = plugins->newInstance<Route>(model, "RouteToPaintMachine1");
        route_to_machine_1->setRouteTimeExpression("2");
        route_to_machine_1->setRouteTimeTimeUnit(Util::TimeUnit::minute);
        route_to_machine_1->setStationName("PaintMachine1");
        route_to_machine_1->setReportStatistics(true);
        
        Route* route_to_machine_2 = plugins->newInstance<Route>(model, "RouteToPaintMachine2");
        route_to_machine_2->setRouteTimeExpression("3");
        route_to_machine_2->setRouteTimeTimeUnit(Util::TimeUnit::minute);
        route_to_machine_2->setStationName("PaintMachine2");
        route_to_machine_2->setReportStatistics(true);
        
        //machining 1
        Resource* blue_painter = plugins->newInstance<Resource>(model, "BluePlainter");
        blue_painter->setCapacity(1);
        
        Queue* machine_1_queue = plugins->newInstance<Queue>(model, "Machine1Process.Queue");
        machine_1_queue->setOrderRule(Queue::OrderRule::FIFO);
        machine_1_queue->setReportStatistics(true);
        
        Enter* machining_1 = plugins->newInstance<Enter>(model, "MachiningArea1");
        machining_1->setStationName("PaintMachine1");
        machining_1->setReportStatistics(true);
        
        Process* machine_1_process = plugins->newInstance<Process>(model, "Machine1Process");
	machine_1_process->getSeizeRequests()->insert(new SeizableItem(blue_painter));
	machine_1_process->setQueueableItem(new QueueableItem(machine_1_queue));
	machine_1_process->setDelayExpression("norm(5.0,0.6)");
        machine_1_process->setDelayTimeUnit(Util::TimeUnit::minute);
        
        Dispose* send_to_dryer_1 = plugins->newInstance<Dispose>(model, "SendPartsToDryer1");
	send_to_dryer_1->setReportStatistics(true);
        
        //machining 2
        Resource* yellow_painter = plugins->newInstance<Resource>(model, "YellowPlainter");
        yellow_painter->setCapacity(1);
        
        Queue* machine_2_queue = plugins->newInstance<Queue>(model, "Machine2Process.Queue");
        machine_2_queue->setOrderRule(Queue::OrderRule::FIFO);
        machine_2_queue->setReportStatistics(true);
        
        Enter* machining_2 = plugins->newInstance<Enter>(model, "MachiningArea2");
        machining_2->setStationName("PaintMachine2");
        machining_2->setReportStatistics(true);
        
        Process* machine_2_process = plugins->newInstance<Process>(model, "Machine2Process");
	machine_2_process->getSeizeRequests()->insert(new SeizableItem(yellow_painter));
	machine_2_process->setQueueableItem(new QueueableItem(machine_2_queue));
	machine_2_process->setDelayExpression("norm(5.0,0.6)");
        machine_2_process->setDelayTimeUnit(Util::TimeUnit::minute);
        
        Dispose* send_to_dryer_2 = plugins->newInstance<Dispose>(model, "SendPartsToDryer2");
	send_to_dryer_2->setReportStatistics(true);
        
        //
	part_arrive->getConnections()->insert(part_arrival);
	part_arrival->getConnections()->insert(route_to_decision);
        
	decision_area->getConnections()->insert(decide);
	decide->getConnections()->insert(route_to_machine_1);
	decide->getConnections()->insert(route_to_machine_2);
        
	machining_1->getConnections()->insert(machine_1_process);
	machine_1_process->getConnections()->insert(send_to_dryer_1);
        
	machining_2->getConnections()->insert(machine_2_process);
	machine_2_process->getConnections()->insert(send_to_dryer_2);
        
        
	// set options, save and simulate
	ModelSimulation* sim = model->getSimulation();
        sim->setWarmUpPeriod(5005.076320*0.05);
        sim->setWarmUpPeriodTimeUnit(Util::TimeUnit::minute);
        sim->setReplicationReportBaseTimeUnit(Util::TimeUnit::minute);
        sim->setNumberOfReplications(300);
	sim->setReplicationLength(std::numeric_limits<double>::max(), Util::TimeUnit::week);
	sim->setTerminatingCondition("(count(SendPartsToDryer1.CountNumberIn)+count(SendPartsToDryer2.CountNumberIn))>1000");
	model->save("./models/Smart_SelectingRouteBasedOnProbability.gen");
	model->getSimulation()->start();
	delete genesys;
	return 0;
	
};

