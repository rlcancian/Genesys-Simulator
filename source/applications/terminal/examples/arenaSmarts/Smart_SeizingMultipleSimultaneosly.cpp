/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Smart_SeizingMultipleSimultaneosly.cpp
 * Author: douglaspereira04
 * 
 */

#include "Smart_SeizingMultipleSimultaneosly.h"

// you have to included need libs

// GEnSyS Simulator
#include "../../../../kernel/simulator/Simulator.h"

// Model Components
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/Dispose.h"
#include "../../../../plugins/components/Seize.h"
#include "../../../../plugins/components/Delay.h"
#include "../../../../plugins/components/Release.h"

#include "../../../../plugins/data/Queue.h"
#include "../../../../plugins/data/Resource.h"
#include "../../../TraitsApp.h"

Smart_SeizingMultipleSimultaneosly::Smart_SeizingMultipleSimultaneosly() {
}

/**
 * This is the main function of the application. 
 * It instanciates the simulator, builds a simulation model and then simulate that model.
 */
int Smart_SeizingMultipleSimultaneosly::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();
	// create model

        //Data Definition
        Resource* operator_ = plugins->newInstance<Resource>(model, "Operator");
        operator_->setCapacity(1);
        operator_->setReportStatistics(true);
        
        Resource* machine = plugins->newInstance<Resource>(model, "Machine");
        machine->setCapacity(2);
        machine->setReportStatistics(true);
        
        Queue* seize_queue = plugins->newInstance<Queue>(model, "SeizeOperatorAndMachine.Queue");
        seize_queue->setOrderRule(Queue::OrderRule::FIFO);
        seize_queue->setReportStatistics(true);
        
        //Create 1
	Create* create_1 = plugins->newInstance<Create>(model, "Create1");
        create_1->setEntityTypeName("Entity1");
        create_1->setTimeBetweenCreationsExpression("expo(8)");
	create_1->setTimeUnit(Util::TimeUnit::minute);
        create_1->setEntitiesPerCreation(1);
        create_1->setFirstCreation(0.0);
        
        //Seize Operator and Machine
        Seize* seize = plugins->newInstance<Seize>(model, "SeizeOperatorAndMachine");
	seize->getSeizeRequests()->insert(new SeizableItem(operator_, "1"));
        seize->getSeizeRequests()->insert(new SeizableItem(machine, "1"));
        seize->setQueue(seize_queue);
        seize->setReportStatistics(true);
        
        //Setup Machine
        Delay* setup_machine = plugins->newInstance<Delay>(model, "SetupMachine");
        setup_machine->setDelayExpression("4", Util::TimeUnit::minute);
        setup_machine->setReportStatistics(true);
        
        //Release Operator
        Release* release_operator = plugins->newInstance<Release>(model, "ReleaseOperator");
	release_operator->getReleaseRequests()->insert(new SeizableItem(operator_, "1"));
        release_operator->setReportStatistics(true);
        
        //Process on Machine
        Delay* process_on_machine = plugins->newInstance<Delay>(model, "ProcessOnMachine");
        process_on_machine->setDelayExpression("4", Util::TimeUnit::minute);
        process_on_machine->setReportStatistics(true);
       
        //Release Machine
        Release* release_machine = plugins->newInstance<Release>(model, "ReleaseMachine");
	release_machine->getReleaseRequests()->insert(new SeizableItem(machine, "1"));
        release_machine->setReportStatistics(true);
        
        //Dispose 1
        Dispose* dispose_1 = plugins->newInstance<Dispose>(model, "Dispose1");
        dispose_1->setReportStatistics(true);
        
	// connect model components to create a "workflow"
	create_1->getConnectionManager()->insert(seize);
        
        seize->getConnectionManager()->insert(setup_machine);
        setup_machine->getConnectionManager()->insert(release_operator);
        
        release_operator->getConnectionManager()->insert(process_on_machine);
        process_on_machine->getConnectionManager()->insert(release_machine);
        
        release_machine->getConnectionManager()->insert(dispose_1);
        
        
	// set options, save and simulate
	ModelSimulation* sim = model->getSimulation();
        sim->setWarmUpPeriod(8001.202613*0.05);
        sim->setWarmUpPeriodTimeUnit(Util::TimeUnit::minute);
	sim->setNumberOfReplications(300);
        sim->setReplicationLength(std::numeric_limits<double>::max(), Util::TimeUnit::week);
	sim->setTerminatingCondition("count(Dispose1.CountNumberIn)>1000");
        sim->setReplicationReportBaseTimeUnit(Util::TimeUnit::minute);
	model->save("./models/Smart_SeizingMultipleSimultaneosly.gen");
	// execute the simulation
	sim->start();
	delete genesys;
	return 0;
};
