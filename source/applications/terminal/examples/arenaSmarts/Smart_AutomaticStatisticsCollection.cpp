/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Smart_AutomaticStatisticsCollection.cpp
 * Author: rlcancian
 * 
 * Created on 3 de Setembro de 2019, 18:34
 */

#include "Smart_AutomaticStatisticsCollection.h"

// you have to included need libs

// GEnSyS Simulator
#include "../../../../kernel/simulator/Simulator.h"

// Model Components
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/Process.h"
#include "../../../../plugins/components/Dispose.h"

Smart_AutomaticStatisticsCollection::Smart_AutomaticStatisticsCollection() {
}

/**
 * This is the main function of the application. 
 * It instanciates the simulator, builds a simulation model and then simulate that model.
 */
int Smart_AutomaticStatisticsCollection::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	this->setDefaultTraceHandlers(genesys->getTracer());
	this->insertFakePluginsByHand(genesys);
        genesys->getTracer()->setTraceLevel(TraceManager::Level::L8_detailed);
	
        // crete model
	Model* model = genesys->getModels()->newModel();
	PluginManager* plugins = genesys->getPlugins();
	
        EntityType* entity1 = plugins->newInstance<EntityType>(model);
        
        Create* create = plugins->newInstance<Create>(model, "Equipment Arrives");
        create->setEntityType(entity1);
        create->setTimeBetweenCreationsExpression("NORM(8,9)");
        create->setTimeUnit(Util::TimeUnit::minute);
        create->setEntitiesPerCreation(1);  // Entities per arrival do arena?
        create->setFirstCreation(0);
        
        Resource* resource = plugins->newInstance<Resource>(model, "Processor");
        resource->setCapacity(1);
        resource->setCostBusyTimeUnit(5.25);
        resource->isReportStatistics();
        
        Process*  process  = plugins->newInstance<Process>(model, "Equipment is Processed");
	process->getSeizeRequests()->insert(new SeizableItem(resource));
	process->setQueueableItem(new QueueableItem(plugins->newInstance<Queue>(model)));
        process->setDelayTimeUnit(Util::TimeUnit::hour);
	process->setDelayExpression("tria(1,2,3)");
        process->setPriority(1);
        process->setAllocationType(Util::AllocationType::ValueAdded);        
        
	Dispose* dispose = plugins->newInstance<Dispose>(model);
	
        // connect model components to create a "workflow"
	create->getConnections()->insert(process);
	process->getConnections()->insert(dispose);
	
        // set options, save and simulate
        ModelSimulation* sim = model->getSimulation();
        
        //sim->setTerminatingCondition("count(Dispose_1.CountNumberIn)>1000");
        sim->setNumberOfReplications(3);
        sim->setReplicationLength(480, Util::TimeUnit::minute);//tem q variar
        sim->setWarmUpPeriod(sim->getReplicationLength()*0.05); //5% de 480 = 24
        sim->setWarmUpPeriodTimeUnit(Util::TimeUnit::minute);
        sim->setReplicationReportBaseTimeUnit(Util::TimeUnit::minute);	
	model->save("./models/Smart_EvaluatingConditionsBeforeEnteringQueue.gen");
	sim->start();
        
        for(int i = 0; i < 1e9; i++);
	delete genesys;
	return 0;
};