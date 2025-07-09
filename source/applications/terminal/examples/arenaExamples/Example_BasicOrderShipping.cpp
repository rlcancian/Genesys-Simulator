/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Example_Basic_Order_Shipping
 * Author: Bruno & Henrique
 * 
 * Created on 2022-2
 */

#include "Example_BasicOrderShipping.h"

// you have to included need libs

// GEnSyS Simulator
#include "../../../../kernel/simulator/Simulator.h"

// Model Components
#include "../../../../plugins/components/DummyComponent.h"
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/Dispose.h"
#include "../../../../plugins/components/Delay.h"
#include "../../../../plugins/components/Process.h"
#include "../../../../plugins/components/Assign.h"
#include "../../../../plugins/components/Decide.h"
#include "../../../../plugins/components/Record.h"
#include "../../../../plugins/data/Queue.h"
#include "../../../../plugins/data/Resource.h"
#include "../../../../plugins/data/Variable.h"

Example_BasicOrderShipping::Example_BasicOrderShipping() {
}

/**
 * This is the main function of the application. 
 * It instantiates the simulator, builds a simulation model and then simulate that model.
 */
int Example_BasicOrderShipping::main(int argc, char** argv) {
	// instantiate simulator
	Simulator* genesys = new Simulator();
	this->setDefaultTraceHandlers(genesys->getTraceManager());
	genesys->getPluginManager()->autoInsertPlugins("autoloadplugins.txt");
	genesys->getTraceManager()->setTraceLevel(TraceManager::Level::L3_errorRecover);
	PluginManager* plugins = genesys->getPluginManager();

	// create model
	Model* model = genesys->getModelManager()->newModel();
	// model->load("model.gen")
	model->getInfos()->setName("Basic Order Shipping");

	// Initialize resources
	Resource* Resource_1 = plugins->newInstance<Resource>(model, "ShippingAgent");
	Resource* Resource_2 = plugins->newInstance<Resource>(model, "AgentReviewer");

	// initialize model parts


	// Create Orders 
	Create* Create_1 = plugins->newInstance<Create>(model, "Orders Arrive for Shipping");
	Create_1->setEntityTypeName("Order");
	Create_1->setTimeBetweenCreationsExpression("expo(50)", Util::TimeUnit::minute);
	Create_1->setFirstCreation(0.001);

	Assign* assign0 = plugins->newInstance<Assign>(model);
	assign0->getAssignments()->insert(new Assignment(model, "", "", true));

	//Process* process1 = plugins->newInstance<Process>(model, "");
	//Process* process2 = plugins->newInstance<Process>(model, "");

	// Process 
	Process* Process2 = plugins->newInstance<Process>(model, "Agent Reviews Order");
	Process2->getSeizeRequests()->insert(new SeizableItem(Resource_2));
	Process2->setDelayExpression("tria(25, 45,60)");
	Process2->setDelayTimeUnit(Util::TimeUnit::minute);
	Process2->setAllocationType(Util::AllocationType::NonValueAdded);

	// Process 
	Process* Process3 = plugins->newInstance<Process>(model, "Order Processing");
	Process3->getSeizeRequests()->insert(new SeizableItem(Resource_1));
	Process3->setDelayExpression("tria(30, 45, 90)");
	Process3->setDelayTimeUnit(Util::TimeUnit::minute);
	Process3->setAllocationType(Util::AllocationType::ValueAdded);

	Decide* decide1 = plugins->newInstance<Decide>(model, "Order Arrives to Costumer?");
	//decide1->
	Decide* decide2 = plugins->newInstance<Decide>(model, "DataOk?");
	//decide1->

	Assign* assign1 = plugins->newInstance<Assign>(model, "Order Leaves 1");
	assign1->getAssignments()->insert(new Assignment(model, "", "", true)); //

	Assign* assign2 = plugins->newInstance<Assign>(model, "Order Leaves 2");
	assign2->getAssignments()->insert(new Assignment(model, "", "", true)); ///

	Assign* assign3 = plugins->newInstance<Assign>(model, "Order Leaves 3");
	assign3->getAssignments()->insert(new Assignment(model, "", "", true)); ///


	Record* record1 = plugins->newInstance<Record>(model, "CycleTime"); //a_time_in with tally name Cycle_time

	Record* record2 = plugins->newInstance<Record>(model, "TotalFailed"); // count totalFailed


	Dispose* dispose1 = plugins->newInstance<Dispose>(model, "Orders Shipping Success");
	Dispose* dispose2 = plugins->newInstance<Dispose>(model, "Orders Shipping Failure");
	Dispose* dispose3 = plugins->newInstance<Dispose>(model, "Invalid Data");

	//create control 
	Create* Create_2 = plugins->newInstance<Create>(model, "Create control entity ");
	Create_2->setEntityTypeName("Entity1");
	Create_2->setTimeBetweenCreationsExpression("expo(1)", Util::TimeUnit::hour);
	Create_2->setMaxCreations(1);
	Create_2->setFirstCreation(0.0);

	Assign* assignDay = plugins->newInstance<Assign>(model); //variable
	assignDay->getAssignments()->insert(new Assignment(model, "v_day", "v_day+1", true)); //@TODO variable eh diff
	Delay* delay24 = plugins->newInstance<Delay>(model);
	delay24->setDelayExpression("24", Util::TimeUnit::hour);
	Decide* decideIf = plugins->newInstance<Decide>(model); //variable v_day
	Assign* reset = plugins->newInstance<Assign>(model);
	reset->getAssignments()->insert(new Assignment(model, "v_day", "0", true));

	//connections

	//control
	Create_2->getConnectionManager()->insert(assignDay);
	assignDay->getConnectionManager()->insert(delay24);
	delay24->getConnectionManager()->insert(decide1);
	decide1->getConnectionManager()->insert(reset, 0); //; TODO two connections f t
	decide1->getConnectionManager()->insert(reset, 1);
	reset->getConnectionManager()->insert(decide1);
	//
	Create_1->getConnectionManager()->insert(assign0);
	assign0->getConnectionManager()->insert(Process2);
	Process2->getConnectionManager()->insert(decide1);
	decide1->getConnectionManager()->insert(Process3, 0);
	decide1->getConnectionManager()->insert(assign1, 1);
	assign1->getConnectionManager()->insert(dispose3); //invalid Data

	Process3->getConnectionManager()->insert(decide2);
	decide2->getConnectionManager()->insert(record1, 0);
	decide2->getConnectionManager()->insert(record2, 1);
	record1->getConnectionManager()->insert(assign3);
	record2->getConnectionManager()->insert(assign2);
	assign3->getConnectionManager()->insert(dispose1); //Orders Shipping Success
	assign2->getConnectionManager()->insert(dispose2);


	// set options, save and simulate
	ModelSimulation* sim = model->getSimulation();
	sim->setNumberOfReplications(3);
	sim->setReplicationLength(4, Util::TimeUnit::day);
	sim->setWarmUpPeriod(48);
	sim->setWarmUpPeriodTimeUnit(Util::TimeUnit::hour);
	//sim->setTerminatingCondition("count(Dispose_1.CountNumberIn)>1000");
	model->save("./models/ExampleBasicOrderShipping.gen");

	// run the simulation
	model->check();
	model->getDataManager()->show();
	sim->start();

	

	// free memory
	delete genesys;
	return 0;
};

