/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Example_PublicTransport
 * Author: Bruno & Henrique
 * 
 * Created on 2022-2
 */

#include "Example_PublicTransport.h"

// you have to included need libs

// GEnSyS Simulator
#include "../../../../kernel/simulator/Simulator.h"

// Model Components
#include "../../../../plugins/components/DummyComponent.h"
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/Dispose.h"
#include "../../../../plugins/components/Delay.h"
#include "../../../../plugins/components/Assign.h"
#include "../../../../plugins/components/Wait.h"
#include "../../../../plugins/components/Route.h"
#include "../../../../plugins/components/Enter.h"
#include "../../../../plugins/components/PickUp.h"
#include "../../../../plugins/components/DropOff.h"
#include "../../../../plugins/components/Search.h"
#include "../../../../plugins/data/EntityGroup.h"
#include "../../../../plugins/data/Station.h"
#include "../../../../plugins/data/Queue.h"
#include "../../../../plugins/data/Variable.h"

Example_PublicTransport::Example_PublicTransport() {
}

/**
 * This is the main function of the application. 
 * It instantiates the simulator, builds a simulation model and then simulate that model.
 */
int Example_PublicTransport::main(int argc, char** argv) {
	// instantiate simulator
	Simulator* genesys = new Simulator();
	this->setDefaultTraceHandlers(genesys->getTraceManager());
	genesys->getPluginManager()->autoInsertPlugins("autoloadplugins.txt");
	genesys->getTraceManager()->setTraceLevel(TraceManager::Level::L3_errorRecover);
	PluginManager* plugins = genesys->getPluginManager();



	// create model
	Model* model = genesys->getModelManager()->newModel();
	// model->load("model.gen")

	model->getInfos()->setName("Public Transport");
	// initialize model parts

	// Create Initial point
	Create* Create_1 = plugins->newInstance<Create>(model, "Initial Point Passengers");
	Create_1->setEntityTypeName("Entity)_1");
	Create_1->setTimeBetweenCreationsExpression("expo(7)", Util::TimeUnit::minute);

	// Create point 2
	Create* Create_2 = plugins->newInstance<Create>(model, "Create Passengers at Point 2");
	Create_2->setEntityTypeName("Entity)_1");
	Create_2->setTimeBetweenCreationsExpression("expo(5)", Util::TimeUnit::minute);

	// Create point 3
	Create* Create_3 = plugins->newInstance<Create>(model, "Create Passengers at Point 3");
	Create_3->setEntityTypeName("Entity)_1");
	Create_3->setTimeBetweenCreationsExpression("expo(8)", Util::TimeUnit::minute);

	// Create BUS
	Create* Create_4 = plugins->newInstance<Create>(model, "Create the Bus");
	Create_4->setEntityTypeName("Entity)_1");
	Create_4->setTimeBetweenCreationsExpression("25", Util::TimeUnit::minute);
	Create_4->setMaxCreations(2);


	Wait* wait1 = plugins->newInstance<Wait>(model);
	wait1->setWaitType(Wait::WaitType::InfiniteHold);
	Queue* queue1 = plugins->newInstance<Queue>(model, "HoldQueue1");
	wait1->setQueue(queue1);

	Wait* wait2 = plugins->newInstance<Wait>(model);
	wait2->setWaitType(Wait::WaitType::InfiniteHold);
	Queue* queue2 = plugins->newInstance<Queue>(model, "HoldQueue2");
	wait2->setQueue(queue2);

	Wait* wait3 = plugins->newInstance<Wait>(model);
	wait3->setWaitType(Wait::WaitType::InfiniteHold);
	Queue* queue3 = plugins->newInstance<Queue>(model, "HoldQueue3");
	wait3->setQueue(queue3);


	Assign* assign1 = plugins->newInstance<Assign>(model);
	assign1->getAssignments()->insert(new Assignment(model, "destination", "DISC(0.3,2,1,3)", true));

	Assign* assign2 = plugins->newInstance<Assign>(model);
	assign1->getAssignments()->insert(new Assignment(model, "destination", "DISC(0.4,3,1,1)", true));

	Assign* assign3 = plugins->newInstance<Assign>(model);
	assign1->getAssignments()->insert(new Assignment(model, "destination", "DISC(0.2,1,1,2)", true));


	Enter* enter0 = plugins->newInstance<Enter>(model);
	enter0->setStationName("Departure_Station");

	Enter* enter1 = plugins->newInstance<Enter>(model);
	enter1->setStationName("InitialPoint");

	Enter* enter2 = plugins->newInstance<Enter>(model);
	enter2->setStationName("Point2");

	Enter* enter3 = plugins->newInstance<Enter>(model);
	enter3->setStationName("Point3");



	Route* route0 = plugins->newInstance<Route>(model);
	route0->setStationName("Departure_Station");
	route0->setRouteTimeTimeUnit(Util::TimeUnit::minute);
	route0->setRouteTimeExpression("expo(3)");


	Route* route2 = plugins->newInstance<Route>(model);
	route2->setStationName("Point2");
	route2->setRouteTimeTimeUnit(Util::TimeUnit::minute);
	route2->setRouteTimeExpression("norm(10,2)");

	Route* route3 = plugins->newInstance<Route>(model);
	route3->setStationName("Point3");
	route3->setRouteTimeTimeUnit(Util::TimeUnit::minute);
	route3->setRouteTimeExpression("norm(20,2)");


	Route* route1 = plugins->newInstance<Route>(model);
	route1->setStationName("InitialPoint");
	route1->setRouteTimeTimeUnit(Util::TimeUnit::minute);
	route1->setRouteTimeExpression("norm(12,2)");


	Search* search1 = plugins->newInstance<Search>(model);
	search1->setSearchCondition("destination==1");
	search1->setStartRank("1");
	search1->setEndRank("NQ(HoldQueue1)-1");
	search1->setSearchIn(queue1);
	//search1->setSearchInType(Search::SearchInType::ENTITYGROUP);
	search1->setSaveFounRankAttribute("rankFound");


	Search* search2 = plugins->newInstance<Search>(model);
	search2->setSearchCondition("destination==2");
	search2->setStartRank("1");
	search2->setEndRank("NQ(HoldQueue2)-1");
	search2->setSearchIn(queue2);
	//search2->setSearchInType(Search::SearchInType::ENTITYGROUP);
	search2->setSaveFounRankAttribute("rankFound");

	Search* search3 = plugins->newInstance<Search>(model);
	search3->setSearchCondition("destination==3");
	search3->setStartRank("1");
	search3->setEndRank("NQ(HoldQueue3)-1");
	search3->setSearchIn(queue3);
	//search3->setSearchInType(Search::SearchInType::ENTITYGROUP);
	search3->setSaveFounRankAttribute("rankFound");

	DropOff* dropOff1 = plugins->newInstance<DropOff>(model);

	DropOff* dropOff2 = plugins->newInstance<DropOff>(model);

	DropOff* dropOff3 = plugins->newInstance<DropOff>(model);

	/*** PickUp TAH DANDO SEGFAULT !!!
//PickUp* pickUp1 = plugins->newInstance<PickUp>(model);
	//pickUp1->setRemoveFrom(wait1);
	//pickUp1->setRemoveStartRank("1");
	//pickUp1->setRemoveEndRank("NG");
        
	//PickUp* pickUp2 = plugins->newInstance<PickUp>(model);
	//pickUp2->setRemoveFrom(wait2);
	//pickUp2->setRemoveStartRank("1");
	//pickUp2->setRemoveEndRank("NG");
        
	//PickUp* pickUp3 = plugins->newInstance<PickUp>(model);
	//pickUp3->setRemoveFrom(wait3);
	//pickUp3->setRemoveStartRank("1");
	//pickUp3->setRemoveEndRank("NG"); 
	 ***/

	Delay* delay1 = plugins->newInstance<Delay>(model);
	delay1->setDelayExpression("TRIA(0.5,1,3)");
	Delay* delay2 = plugins->newInstance<Delay>(model);
	delay2->setDelayExpression("TRIA(0.5,1,3)");
	Delay* delay3 = plugins->newInstance<Delay>(model);
	delay3->setDelayExpression("TRIA(0.5,1,3)");


	Dispose* dispose1 = plugins->newInstance<Dispose>(model);
	dispose1->setReportStatistics(true);
	// connect model components


	Create_1->getConnectionManager()->insert(assign1);
	Create_2->getConnectionManager()->insert(assign2);
	Create_3->getConnectionManager()->insert(assign3);
	Create_4->getConnectionManager()->insert(search1);

	assign1->getConnectionManager()->insert(wait1);
	assign2->getConnectionManager()->insert(wait2);
	assign3->getConnectionManager()->insert(wait3);

	wait1->getConnectionManager()->insert(wait1);
	wait2->getConnectionManager()->insert(wait2);
	wait3->getConnectionManager()->insert(wait3);
	//
	enter1->getConnectionManager()->insert(search1); //"InitialPoint");
	enter2->getConnectionManager()->insert(search2); //"Point2");
	enter3->getConnectionManager()->insert(search3); //"Point3");

	search1->getConnectionManager()->insert(dropOff1); //"InitialPoint");
	search2->getConnectionManager()->insert(dropOff2); //"Point2");
	search3->getConnectionManager()->insert(dropOff3); //"Point3");

	dropOff1->getConnectionManager()->insert(search1); //"InitialPoint");
	dropOff2->getConnectionManager()->insert(search2); //"Point2");
	dropOff3->getConnectionManager()->insert(search3); //"Point3");

	dropOff1->getConnectionManager()->insert(route0); //"InitialPoint");
	dropOff2->getConnectionManager()->insert(route0); //"Point2");
	dropOff3->getConnectionManager()->insert(route0); //"Point3");

	/*
	search1->getConnections()->insert(pickUp1); //"InitialPoint");
	search2->getConnections()->insert(pickUp2); //"Point2");
	search3->getConnections()->insert(pickUp3); //"Point3");
        
	pickUp1->getConnections()->insert(delay1); //"InitialPoint");
	pickUp2->getConnections()->insert(delay2); //"Point2");
	pickUp3->getConnections()->insert(delay3); //"Point3");
        
	test workaround segfault*/
	search1->getConnectionManager()->insert(delay1); //"InitialPoint");
	search2->getConnectionManager()->insert(delay2); //"Point2");
	search3->getConnectionManager()->insert(delay3); //"Point3");


	delay1->getConnectionManager()->insert(route2); // local "InitialPoint");
	delay2->getConnectionManager()->insert(route3); // local "Point2");
	delay3->getConnectionManager()->insert(route1); // local "Point3");


	enter0->getConnectionManager()->insert(dispose1);

	// set options, save and simulate
	ModelSimulation* sim = model->getSimulation();
	sim->setNumberOfReplications(3);
	sim->setReplicationLength(4, Util::TimeUnit::hour);
	sim->setWarmUpPeriod(0.2);
	sim->setWarmUpPeriodTimeUnit(Util::TimeUnit::hour);
	//sim->setTerminatingCondition("count(Dispose_1.CountNumberIn)>1000");
	model->save("./models/Example_PublicTransport.gen");

	model->getTracer()->setTraceLevel(TraceManager::Level::L8_detailed);
	// run the simulation
	model->check();
	model->getDataManager()->show();
	sim->start();

	
	// free memory
	delete genesys;
	return 0;
};

