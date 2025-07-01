/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Smart_HoldSearchRemove.cpp
 * Author: rlcancian
 *
 * Created on 01 de Dezembro de 2022, 09:31
 */

#include "Smart_HoldSearchRemove.h"

// you have to included need libs

// GEnSyS Simulator
#include "../../../../kernel/simulator/Simulator.h"

// Model Components
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/Assign.h"
#include "../../../../plugins/components/Wait.h"
#include "../../../../plugins/components/Search.h"
#include "../../../../plugins/components/Remove.h"
#include "../../../../plugins/components/Dispose.h"

Smart_HoldSearchRemove::Smart_HoldSearchRemove() {
}

/**
 * This is the main function of the application. 
 * It instanciates the simulator, builds a simulation model and then simulate that model.
 */
int Smart_HoldSearchRemove::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	//genesys->getTracer()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();
	// crete model
	Create* create1 = plugins->newInstance<Create>(model);
	Create* create2 = plugins->newInstance<Create>(model);
	Assign* assign1 = plugins->newInstance<Assign>(model);
	assign1->getAssignments()->insert(new Assignment(model, "vid", "vid+1", false));
	assign1->getAssignments()->insert(new Assignment(model, "id", "vid", true));
	Queue* queue1 =  plugins->newInstance<Queue>(model, "HoldQueue");
	Wait* hold1 = plugins->newInstance<Wait>(model);
	hold1->setQueue(queue1);
	hold1->setWaitType(Wait::WaitType::InfiniteHold);
	Search* search1 = plugins->newInstance<Search>(model);
	search1->setSearchIn(queue1);
	search1->setSearchCondition("id>5");
	search1->setSaveFounRankAttribute("rankFound");
	search1->setStartRank("2");
	search1->setEndRank("NQ(HoldQueue)-1");
	Remove* remove1 = plugins->newInstance<Remove>(model);
	remove1->setRemoveFrom(queue1);
	remove1->setRemoveStartRank("rankFound");
	Dispose* dispose1 = plugins->newInstance<Dispose>(model);
	Dispose* dispose2 = plugins->newInstance<Dispose>(model);
	Dispose* dispose3 = plugins->newInstance<Dispose>(model, "No entity will ever arrive here");
	// connect model components to create a "workflow"
	create1->getConnections()->insert(assign1);
	assign1->getConnections()->insert(hold1);
	hold1->getConnections()->insert(dispose3); // just because "Wait" is not a SinkComponent
	create2->getConnections()->insert(search1);
	search1->getConnections()->insert(dispose1);
	search1->getConnections()->insert(remove1);
	remove1->getConnections()->insert(dispose1);
	remove1->getConnections()->insert(dispose2);
	// set options, save and simulate
	model->getSimulation()->setReplicationLength(10, Util::TimeUnit::second);
	model->save("./models/Smart_HoldSearchRemove.gen");
	model->check();
	do {
		model->getSimulation()->step();
	} while (model->getSimulation()->isPaused());
	delete genesys;
	return 0;
};

