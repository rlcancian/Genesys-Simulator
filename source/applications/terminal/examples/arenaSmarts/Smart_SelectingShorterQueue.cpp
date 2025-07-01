/*
 * File:   SelectingShorterQueue.cpp
 * Author: genesys
 *
 * Created on 10 de dezembro de 2022, 17:58
 */

#include "Smart_SelectingShorterQueue.h"

// you have to included need libs

// GEnSyS Simulator
#include "../../../../kernel/simulator/Simulator.h"

// Model Components
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/Dispose.h"
#include "../../../../plugins/components/Process.h"
#include "../../../../plugins/components/Decide.h"
#include "../../../TraitsApp.h"

Smart_SelectingShorterQueue::Smart_SelectingShorterQueue() {
}

/**
 * This is the main function of the application. 
 * It instanciates the simulator, builds a simulation model and then simulate that model.
 */
int Smart_SelectingShorterQueue::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();
	// create model

        //create 1
        // entities per arrival padrão: 1, max arrivals padrão: infinite, first_creation padrão: 0.0
	Create* create1 = plugins->newInstance<Create>(model);
        create1->setEntityTypeName("Create 1");
	create1->setTimeBetweenCreationsExpression("3");
	create1->setTimeUnit(Util::TimeUnit::minute);
        
        // decide 1
        Decide* decide1 = plugins->newInstance<Decide>(model);
        // arrumar as condições
        decide1->getConditions()->insert("NQ(QueueResource1) < NQ(QueueResource2)");
        
        // process one
        Process* process1 = plugins->newInstance<Process>(model);
        process1->setName("Process 1");
        process1->setDelayExpression("NORM(4,4)");
        process1->setDelayTimeUnit(Util::TimeUnit::minute);
        process1->setAllocationType(Util::AllocationType::ValueAdded);
        
        
        // process two
        Process* process2 = plugins->newInstance<Process>(model);
        process2->setName("Process 2");
        process2->setDelayExpression("NORM(4,4)");
        process2->setDelayTimeUnit(Util::TimeUnit::minute);
        process2->setAllocationType(Util::AllocationType::ValueAdded);
        
        
        //Create resources
        Resource* resource1 = plugins->newInstance<Resource>(model, "Machine 1");
        resource1->setCapacity(1);
        
        Resource* resource2 = plugins->newInstance<Resource>(model, "Machine 2");
        resource2->setCapacity(1);
        
        //
        process1->getSeizeRequests()->insert(new SeizableItem(resource1, "1"));
	process1->setQueueableItem(new QueueableItem(model, "QueueResource1"));
        
        process2->getSeizeRequests()->insert(new SeizableItem(resource2, "1"));
	process2->setQueueableItem(new QueueableItem(model, "QueueResource2"));
        
        // dispose 1
        Dispose* dispose1 = plugins->newInstance<Dispose>(model);
        
        // CONNECTIONS
        create1->getConnections()->insert(decide1);
        // insere primeiro o machineOne para que o resultado da expressão vá para ele caso seja True
        decide1->getConnections()->insert(process1);
        // se o resultado da expressão for false irá para Machine Two
        decide1->getConnections()->insert(process2);
        
        process1->getConnections()->insert(dispose1);
        process2->getConnections()->insert(dispose1);
	// set options, save and simulate
	model->getSimulation()->setNumberOfReplications(300);
        model->getSimulation()->setReplicationReportBaseTimeUnit(Util::TimeUnit::minute);
        model->getSimulation()->setReplicationLength(3200, Util::TimeUnit::minute); // tempo que dura uma replicação
        
        // warm up
	model->getSimulation()->setWarmUpPeriod(model->getSimulation()->getReplicationLength()*0.05);
	model->getSimulation()->setWarmUpPeriodTimeUnit(Util::TimeUnit::minute);
        
	model->save("./models/Smart_SelectingShorterQueue.gen");
	model->getSimulation()->start();
	delete genesys;
	return 0;
};
