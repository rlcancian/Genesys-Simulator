/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Smart_WaitForSignal.cpp
 * Author: douglaspereira04
 * 
 */

#include "Smart_WaitForSignal.h"

// you have to included need libs

// GEnSyS Simulator
#include "../../../../kernel/simulator/Simulator.h"

// Model Components
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/Process.h"
#include "../../../../plugins/components/Wait.h"
#include "../../../../plugins/components/Signal.h"
#include "../../../../plugins/components/Dispose.h"

#include "../../../../plugins/data/SignalData.h"
#include "../../../../plugins/data/Resource.h"
#include "../../../TraitsApp.h"

Smart_WaitForSignal::Smart_WaitForSignal() {
}

/**
 * This is the main function of the application. 
 * It instanciates the simulator, builds a simulation model and then simulate that model.
 */
int Smart_WaitForSignal::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();
	// create model

        //Data Definition        
	SignalData* signal_data_1 = plugins->newInstance<SignalData>(model, "Signal1");
        
        Resource* processor = plugins->newInstance<Resource>(model, "Processor");
        processor->setCapacity(1);
        processor->setReportStatistics(true);
        
        Queue* hold_1_queue = plugins->newInstance<Queue>(model, "Hold1.Queue");
        hold_1_queue->setOrderRule(Queue::OrderRule::FIFO);
        hold_1_queue->setReportStatistics(true);
        
        //Create 1
	Create* create_1 = plugins->newInstance<Create>(model, "Create1");
        create_1->setEntityTypeName("Entity1");
	create_1->setTimeBetweenCreationsExpression("expo(10)");
	create_1->setTimeUnit(Util::TimeUnit::minute);
	create_1->setEntitiesPerCreation(1);
        create_1->setMaxCreations(50);
	create_1->setFirstCreation(0.0);
        
        //Create 2
	Create* create_2 = plugins->newInstance<Create>(model, "Create2");
        create_2->setEntityTypeName("Entity1");
	create_2->setTimeBetweenCreationsExpression("60");
	create_2->setTimeUnit(Util::TimeUnit::minute);
	create_2->setEntitiesPerCreation(1);
        create_2->setMaxCreations(50);
	create_2->setFirstCreation(0.0);
        
        //Hold 1
	Wait* hold_1 = plugins->newInstance<Wait>(model, "Hold1");
	hold_1->setSignalData(signal_data_1);
        hold_1->setQueue(hold_1_queue);
        
        //Signal 1
	Signal* signal_1 = plugins->newInstance<Signal>(model, "Signal1");
	signal_1->setSignalData(signal_data_1);
	signal_1->setLimitExpression("1");
        
        //Process 1
        Process* process_1 = plugins->newInstance<Process>(model, "Process1");
	process_1->getSeizeRequests()->insert(new SeizableItem(processor));
	process_1->setQueueableItem(new QueueableItem(plugins->newInstance<Queue>(model)));
        process_1->setAllocationType(Util::AllocationType::ValueAdded);
	process_1->setDelayExpression("unif(120.0,240.0)");
        process_1->setDelayTimeUnit(Util::TimeUnit::minute);
        process_1->setReportStatistics(true);
	
        //Dispose 2
        Dispose* dispose_1 = plugins->newInstance<Dispose>(model, "Dispose1");
        dispose_1->setReportStatistics(true);
        
        //Dispose 2
        Dispose* dispose_2 = plugins->newInstance<Dispose>(model, "Dispose2");
        dispose_2->setReportStatistics(true);
        
	// connect model components to create a "workflow"
	create_1->getConnectionManager()->insert(hold_1);
	hold_1->getConnectionManager()->insert(process_1);
	process_1->getConnectionManager()->insert(dispose_1);
        
	create_2->getConnectionManager()->insert(signal_1);
	signal_1->getConnectionManager()->insert(dispose_2);
                
        
	// set options, save and simulate
	ModelSimulation* sim = model->getSimulation();
        sim->setWarmUpPeriod(9240.154287*0.05);
        sim->setWarmUpPeriodTimeUnit(Util::TimeUnit::minute);
	sim->setNumberOfReplications(300);
        sim->setReplicationLength(std::numeric_limits<double>::max(), Util::TimeUnit::week);
	sim->setTerminatingCondition("(count(Dispose1.CountNumberIn)+count(Dispose2.CountNumberIn))>1000");
        sim->setReplicationReportBaseTimeUnit(Util::TimeUnit::minute);
	model->save("./models/Smart_WaitForSignal.gen");
	// execute the simulation
	sim->start();
	delete genesys;
	return 0;
};
