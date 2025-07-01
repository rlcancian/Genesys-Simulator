/* 
 * File:   AddingResource.cpp
 * Author: genesys
 *
 * Created on 10 de dezembro de 2022, 17:58
 */

#include "Smart_AddingResource.h"

// you have to included need libs

// GEnSyS Simulator
#include "../../../../kernel/simulator/Simulator.h"

// Model Components
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/Dispose.h"
#include "../../../../plugins/components/Process.h"
#include "../../../TraitsApp.h"

Smart_AddingResource::Smart_AddingResource() {
}

/**
 * This is the main function of the application. 
 * It instanciates the simulator, builds a simulation model and then simulate that model.
 */
int Smart_AddingResource::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();
	// create model
	Create* create1 = plugins->newInstance<Create>(model);
	create1->setEntityTypeName("Create 1");
	create1->setTimeBetweenCreationsExpression("expo(2)");
	create1->setTimeUnit(Util::TimeUnit::minute);

	// process 1
	// seize delay release
	// o setAllocation é em todos ou só em 1?
	Process* process1 = plugins->newInstance<Process>(model);
	process1->setName("Process 1");
	process1->setDelayExpression("TRIA(0.5,1.0,1.5)");
	process1->setDelayTimeUnit(Util::TimeUnit::minute);
	process1->setAllocationType(Util::AllocationType::ValueAdded);

	// resource dentro do process, ainda ver o que é isso
	Resource* resource1 = plugins->newInstance<Resource>(model, "Resource 1");
	resource1->setCapacity(1);

	process1->getSeizeRequests()->insert(new SeizableItem(resource1, "1"));
	process1->setQueueableItem(new QueueableItem(model, "QueueResource1"));

	// dispose 1
	Dispose* dispose1 = plugins->newInstance<Dispose>(model);

	//connections
	create1->getConnections()->insert(process1);
	process1->getConnections()->insert(dispose1);
	// set options, save and simulate

	model->getSimulation()->setNumberOfReplications(3);
	model->getSimulation()->setReplicationReportBaseTimeUnit(Util::TimeUnit::minute);
	model->getSimulation()->setReplicationLength(24, Util::TimeUnit::minute); // tempo que dura uma replicação

	// warm up
	model->getSimulation()->setWarmUpPeriod(model->getSimulation()->getReplicationLength()*0.05);
	model->getSimulation()->setWarmUpPeriodTimeUnit(Util::TimeUnit::minute);

	model->save("./models/Smart_AddingResource.gen");
	model->getSimulation()->start();
	delete genesys;
	return 0;
};
