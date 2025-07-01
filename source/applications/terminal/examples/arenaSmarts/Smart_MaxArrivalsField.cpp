/* 
 * File:   Smart_MaxArrivalsField.cpp
 * Author: genesys
 *
 * Created on 10 de dezembro de 2022, 15:35
 */

#include "Smart_MaxArrivalsField.h"

// you have to included need libs

// GEnSyS Simulator
#include "../../../../kernel/simulator/Simulator.h"

// Model Components
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/Dispose.h"
#include "../../../../plugins/components/Record.h"
#include "../../../TraitsApp.h"

Smart_MaxArrivalsField::Smart_MaxArrivalsField() {
}

/**
 * This is the main function of the application. 
 * It instanciates the simulator, builds a simulation model and then simulate that model.
 */
int Smart_MaxArrivalsField::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();
	// create model

        // create1
	Create* create1 = plugins->newInstance<Create>(model);
        create1->setMaxCreations("MAX ( (TNOW < 120) * 10000 , (TNOW >= 120) * 1 )");
//        create1->setMaxCreations("100000000000");
//        create1->setMaxCreations("MAX ( 100000 , 20 ) ");
        create1->setEntityTypeName("Create1");
	create1->setTimeBetweenCreationsExpression("expo(2)");
	create1->setTimeUnit(Util::TimeUnit::minute);
//        
//        Counter* c = plugins->newInstance<Counter>(model);
//        c->setName("counter1");
        
        // record How Many
        Record* howMany = plugins->newInstance<Record>(model);
        howMany->setName("HowMany");
        
        // dispose
	Dispose* dispose1 = plugins->newInstance<Dispose>(model);
        
	// connect model components to create a "workflow"
	create1->getConnections()->insert(howMany);
	howMany->getConnections()->insert(dispose1);
	// set options, save and simulate
	model->getSimulation()->setNumberOfReplications(300);
        model->getSimulation()->setReplicationReportBaseTimeUnit(Util::TimeUnit::minute);
        model->getSimulation()->setReplicationLength(120, Util::TimeUnit::minute); // tempo que dura uma replicação
        
        // warm up
	model->getSimulation()->setWarmUpPeriod(model->getSimulation()->getReplicationLength()*0.05);
	model->getSimulation()->setWarmUpPeriodTimeUnit(Util::TimeUnit::minute);
        
	model->save("./models/Smart_MaxArrivalsField.gen");
	model->getSimulation()->start();
        for(int i=0; i<1e9; i++);
	delete genesys;
	return 0;
};
