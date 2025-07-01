/* 
 * File:   Smart_MaxArrivalsField.cpp
 * Author: genesys
 *
 * Created on 10 de dezembro de 2022, 15:35
 */

#include "Smart_DefiningAttributesAsStrings.h"
// you have to included need libs

// GEnSyS Simulator
#include "../../../../kernel/simulator/Simulator.h"

// Model Components
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/Dispose.h"
#include "../../../../plugins/components/Assign.h"
#include "../../../../plugins/components/Record.h"
#include "../../../../plugins/components/Delay.h"
#include "../../../../plugins/components/Decide.h"
#include "../../../TraitsApp.h"


Smart_DefiningAttributesAsStrings::Smart_DefiningAttributesAsStrings() {
}

/**
 * This is the main function of the application. 
 * It instanciates the simulator, builds a simulation model and then simulate that model.
 */
int Smart_DefiningAttributesAsStrings::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();
	// create model
		// create 1
	Create* create1 = plugins->newInstance<Create>(model);
        create1->setEntityTypeName("Create 1");
	create1->setTimeBetweenCreationsExpression("expo(1)");
	create1->setTimeUnit(Util::TimeUnit::hour);
        
        // assign 1
        Assign* assign1 = plugins->newInstance<Assign>(model);
        plugins->newInstance<Attribute>(model, "myatt");
        assign1->getAssignments()->insert(new Assignment("myatt", "1")); // 1 == blue
        
        
        // create 3
        Create* create3 = plugins->newInstance<Create>(model);
        create3->setEntityTypeName("Create 3");
	create3->setTimeBetweenCreationsExpression("expo(1)");
	create3->setTimeUnit(Util::TimeUnit::hour);
        
        
        Decide* decide1 = plugins->newInstance<Decide>(model);
        decide1->getConditions()->insert("myatt == 1");
        
        // process 1 e 2
        Delay* delay1 = plugins->newInstance<Delay>(model);
        delay1->setName("Process 1");
        delay1->setDelayExpression("TRIA(0.5,1.0,1.5)");
        delay1->setDelayTimeUnit(Util::TimeUnit::hour);
        delay1->setAllocation(Util::AllocationType::ValueAdded);
        
        Delay* delay2 = plugins->newInstance<Delay>(model);
        delay2->setName("Process 2");
        delay2->setDelayExpression("TRIA(0.5,1.0,1.5)");
        delay2->setDelayTimeUnit(Util::TimeUnit::hour);
        delay2->setAllocation(Util::AllocationType::ValueAdded);
        
        // dispose
        Dispose* dispose1 = plugins->newInstance<Dispose>(model);
        
        // connections
        create1->getConnections()->insert(assign1);
        create3->getConnections()->insert(decide1);
        assign1->getConnections()->insert(decide1);
        // true go to process 1
        decide1->getConnections()->insert(delay1);
        // false go to process 2
        decide1->getConnections()->insert(delay2);
        delay1->getConnections()->insert(dispose1);
        delay2->getConnections()->insert(dispose1);
	
        
	// set options, save and simulate
	model->getSimulation()->setNumberOfReplications(300);
        model->getSimulation()->setReplicationReportBaseTimeUnit(Util::TimeUnit::minute);
        model->getSimulation()->setReplicationLength(38000, Util::TimeUnit::minute); // tempo que dura uma replicação
        
        // warm up
	model->getSimulation()->setWarmUpPeriod(model->getSimulation()->getReplicationLength()*0.05);
	model->getSimulation()->setWarmUpPeriodTimeUnit(Util::TimeUnit::minute);
        
        
	model->save("./models/Smart_DefiningAttributesAsStrings.gen");
	model->getSimulation()->start();
	delete genesys;
	return 0;
};
