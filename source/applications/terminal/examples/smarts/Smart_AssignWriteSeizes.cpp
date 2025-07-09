/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Smart_AssignWriteSeizes.cpp
 * Author: rlcancian
 * 
 * Created on 24 de Setembro de 2019, 20:56
 */

#include "Smart_AssignWriteSeizes.h"

// GEnSyS Simulator
#include "../../../../kernel/simulator/Simulator.h"

// Model Components
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/Delay.h"
#include "../../../../plugins/components/Dispose.h"
#include "../../../../plugins/components/Seize.h"
#include "../../../../plugins/components/Release.h"
#include "../../../../plugins/components/Assign.h"
#include "../../../../plugins/components/Record.h"
#include "../../../../plugins/components/Decide.h"
#include "../../../../plugins/components/Write.h"

// Model data definitions
#include "../../../../kernel/simulator/ModelDataManager.h"
#include "../../../../plugins/data/Set.h"
#include "../../../TraitsApp.h"

Smart_AssignWriteSeizes::Smart_AssignWriteSeizes() {
}

int Smart_AssignWriteSeizes::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();
	// create model
	Create* create1 = plugins->newInstance<Create>(model);
	create1->setEntityTypeName("Part");
	create1->setTimeBetweenCreationsExpression("norm(1.5,0.5)", Util::TimeUnit::second);
	create1->setEntitiesPerCreation(1);
	Assign* assign1 = plugins->newInstance<Assign>(model);
	assign1->getAssignments()->insert(new Assignment(model, "varNextIndex", "varNextIndex + 1", false));
	assign1->getAssignments()->insert(new Assignment(model, "index", "varNextIndex", true));
	Write* write1 = plugins->newInstance<Write>(model);
	write1->setWriteToType(Write::WriteToType::SCREEN);
	write1->insertText({"\n", "Atributo index: ", "'index'"});
	write1->insertText({"Variável nextIndex: ", "'varNextIndex'"});
	//
	Resource* machine1 = plugins->newInstance<Resource>(model, "Machine_1");
	machine1->setCapacity(1);
	Resource* machine2 = plugins->newInstance<Resource>(model, "Machine_2");
	machine2->setCapacity(2);
	Resource* machine3 = plugins->newInstance<Resource>(model, "Machine_3");
	machine3->setCapacity(3);
	Set* machSet = plugins->newInstance<Set>(model, "Machine_Set");
	machSet->setSetOfType(Util::TypeOf<Resource>());
	machSet->getElementSet()->insert(machine1);
	machSet->getElementSet()->insert(machine2);
	machSet->getElementSet()->insert(machine3);
	Decide* decide1 = plugins->newInstance<Decide>(model);
	decide1->getConditions()->insert("NR(Machine_1) < MR(Machine_1)");
	decide1->getConditions()->insert("NR(Machine_2) < MR(Machine_2)");
	Queue* queueSeize1 = plugins->newInstance<Queue>(model, "Queue_Seize_1");
	queueSeize1->setOrderRule(Queue::OrderRule::FIFO);
	Seize* seize1 = plugins->newInstance<Seize>(model);
	seize1->getSeizeRequests()->insert(new SeizableItem(machine1));
	seize1->setQueue(queueSeize1);
	Delay* delay1 = plugins->newInstance<Delay>(model);
	delay1->setDelayExpression("norm(15,1)");
	delay1->setDelayTimeUnit(Util::TimeUnit::second);
	Release* release1 = plugins->newInstance<Release>(model);
	release1->getReleaseRequests()->insert(new SeizableItem(machine1));
	Queue* queueSeize2 = plugins->newInstance<Queue>(model, "Queue_Seize_2");
	queueSeize2->setOrderRule(Queue::OrderRule::FIFO);
	Seize* seize2 = plugins->newInstance<Seize>(model);
	seize2->getSeizeRequests()->insert(new SeizableItem(machine2));
	seize2->setQueue(queueSeize2);
	Delay* delay2 = plugins->newInstance<Delay>(model);
	delay2->setDelayExpression("norm(15,1)", Util::TimeUnit::second);
	Release* release2 = plugins->newInstance<Release>(model);
	release2->getReleaseRequests()->insert(new SeizableItem(machine2));
	Queue* queueSeize3 = plugins->newInstance<Queue>(model, "Queue_Seize_3");
	queueSeize3->setOrderRule(Queue::OrderRule::FIFO);
	Seize* seize3 = plugins->newInstance<Seize>(model);
	seize3->getSeizeRequests()->insert(new SeizableItem(machine3));
	seize3->setQueue(queueSeize3);
	Delay* delay3 = plugins->newInstance<Delay>(model);
	delay3->setDelayExpression("norm(15,1)", Util::TimeUnit::second);
	Release* release3 = plugins->newInstance<Release>(model);
	release3->getReleaseRequests()->insert(new SeizableItem(machine3));
	Dispose* dispose1 = plugins->newInstance<Dispose>(model);
	// connect model components to create a "workflow"
	create1->getConnectionManager()->insert(assign1);
	assign1->getConnectionManager()->insert(write1);
	write1->getConnectionManager()->insert(decide1);
	decide1->getConnectionManager()->insert(seize1);
	decide1->getConnectionManager()->insert(seize2);
	decide1->getConnectionManager()->insert(seize3);
	seize1->getConnectionManager()->insert(delay1);
	delay1->getConnectionManager()->insert(release1);
	release1->getConnectionManager()->insert(dispose1);
	seize2->getConnectionManager()->insert(delay2);
	delay2->getConnectionManager()->insert(release2);
	release2->getConnectionManager()->insert(dispose1);
	seize3->getConnectionManager()->insert(delay3);
	delay3->getConnectionManager()->insert(release3);
	release3->getConnectionManager()->insert(dispose1);
	// save and run
	ModelSimulation* sim = model->getSimulation();
	sim->setNumberOfReplications(5);
	sim->setReplicationLength(100);
	model->save("./models/Smart_AssignWriteSeizes.gen");
	sim->start();
	delete genesys;
	return 0;
}

