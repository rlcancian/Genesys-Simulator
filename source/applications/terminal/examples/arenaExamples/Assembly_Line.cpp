/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Smart_Delay.cpp
 * Author: rlcancian
 * 
 * Created on 3 de Setembro de 2019, 18:34
 */

#include "Assembly_Line.h"

// you have to included need libs

// GEnSyS Simulator
#include "../../../../kernel/simulator/Simulator.h"

// Model Components
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/Dispose.h"
#include "../../../../plugins/components/Assign.h"
#include "../../../../plugins/components/Delay.h"
#include "../../../../plugins/components/Seize.h"
#include "../../../../plugins/components/Enter.h"
#include "../../../../plugins/components/Release.h"
#include "../../../../plugins/data/Station.h"
#include "../../../../plugins/data/Resource.h"
#include "../../../../plugins/components/Decide.h"
#include "../../../../plugins/components/Record.h"
#include "../../../../plugins/components/Route.h"

Assembly_Line::Assembly_Line() {
}

/**
 * This is the main function of the application. 
 * It instanciates the simulator, builds a simulation model and then simulate that model.
 */
int Assembly_Line::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	this->setDefaultTraceHandlers(genesys->getTraceManager());
	genesys->getPluginManager()->autoInsertPlugins("autoloadplugins.txt");
	// create model
	Model* model = genesys->getModelManager()->newModel();
	PluginManager* plugins = genesys->getPluginManager();

	//Inicio
	Create* create1 = plugins->newInstance<Create>(model);
	create1->setEntityTypeName("Peca");
	create1->setTimeBetweenCreationsExpression("TRIA(0,3,7)", Util::TimeUnit::minute);
	create1->setEntitiesPerCreation(1);

	plugins->newInstance<Attribute>(model, "a_time");
	plugins->newInstance<Variable>(model, "v_WIP");
	plugins->newInstance<Variable>(model, "countProducts");
	Assign* assign1 = plugins->newInstance<Assign>(model);
	assign1->getAssignments()->insert(new Assignment("a_time", "TNOW"));
	assign1->getAssignments()->insert(new Assignment("v_WIP", "v_WIP+1"));

	create1->getConnectionManager()->insert(assign1);

	Station* entranceStation = plugins->newInstance<Station>(model, "Estacao_de_Entrada");
	Enter* enterEntranceStation = plugins->newInstance<Enter>(model);
	enterEntranceStation->setStation(entranceStation);

	assign1->getConnectionManager()->insert(enterEntranceStation);

	Station* station1 = plugins->newInstance<Station>(model, "Estacao_1");
	Enter* enterStation1 = plugins->newInstance<Enter>(model);
	enterStation1->setStation(station1);
	Route* route1 = plugins->newInstance<Route>(model);
	route1->setStation(station1);
	route1->setRouteTimeExpression("30");
	route1->setRouteTimeTimeUnit(Util::TimeUnit::second);

	enterEntranceStation->getConnectionManager()->insert(route1);

	Resource* operator1 = plugins->newInstance<Resource>(model, "operador_1");
	operator1->setCapacity(1);
	Queue* queueOperator1 = plugins->newInstance<Queue>(model, "Fila_Operador1");
	Seize* seizeOperator1 = plugins->newInstance<Seize>(model);
	seizeOperator1->setQueue(queueOperator1);
	seizeOperator1->getSeizeRequests()->insert(new SeizableItem(operator1, "1"));
	Delay* delayOperator1 = plugins->newInstance<Delay>(model);
	delayOperator1->setDelayExpression("NORM(2,1)");
	delayOperator1->setDelayTimeUnit(Util::TimeUnit::minute);
	Release* releaseOperator1 = plugins->newInstance<Release>(model);
	releaseOperator1->getReleaseRequests()->insert(new SeizableItem(operator1, "1"));

	enterStation1->getConnectionManager()->insert(seizeOperator1);
	seizeOperator1->getConnectionManager()->insert(delayOperator1);
	delayOperator1->getConnectionManager()->insert(releaseOperator1);

	Station* station2 = plugins->newInstance<Station>(model, "Estacao_2");
	Enter* enterStation2 = plugins->newInstance<Enter>(model);
	enterStation2->setStation(station2);
	Route* route2 = plugins->newInstance<Route>(model);
	route2->setStation(station2);
	route2->setRouteTimeExpression("40");
	route2->setRouteTimeTimeUnit(Util::TimeUnit::second);

	releaseOperator1->getConnectionManager()->insert(route2);

	Resource* operator2 = plugins->newInstance<Resource>(model, "operador_2");
	operator2->setCapacity(1);
	Queue* queueOperator2 = plugins->newInstance<Queue>(model, "Fila_Operador2");
	Seize* seizeOperator2 = plugins->newInstance<Seize>(model);
	seizeOperator2->setQueue(queueOperator2);
	seizeOperator2->getSeizeRequests()->insert(new SeizableItem(operator2, "1"));
	Delay* delayOperator2 = plugins->newInstance<Delay>(model);
	delayOperator2->setDelayExpression("NORM(2.5,1)");
	delayOperator2->setDelayTimeUnit(Util::TimeUnit::minute);
	Release* releaseOperator2 = plugins->newInstance<Release>(model);
	releaseOperator2->getReleaseRequests()->insert(new SeizableItem(operator2, "1"));

	enterStation2->getConnectionManager()->insert(seizeOperator2);
	seizeOperator2->getConnectionManager()->insert(delayOperator2);
	delayOperator2->getConnectionManager()->insert(releaseOperator2);

	Station* station3 = plugins->newInstance<Station>(model, "Estacao_3");
	Enter* enterStation3 = plugins->newInstance<Enter>(model);
	enterStation3->setStation(station3);
	Route* route3 = plugins->newInstance<Route>(model);
	route3->setStation(station3);
	route3->setRouteTimeExpression("60");
	route3->setRouteTimeTimeUnit(Util::TimeUnit::second);

	releaseOperator2->getConnectionManager()->insert(route3);

	Resource* operator3 = plugins->newInstance<Resource>(model, "operador_3");
	operator3->setCapacity(1);
	Queue* queueOperator3 = plugins->newInstance<Queue>(model, "Fila_Operador3");
	Seize* seizeOperator3 = plugins->newInstance<Seize>(model);
	seizeOperator3->setQueue(queueOperator3);
	seizeOperator3->getSeizeRequests()->insert(new SeizableItem(operator3, "1"));
	Delay* delayOperator3 = plugins->newInstance<Delay>(model);
	delayOperator3->setDelayExpression("NORM(2.5,1)");
	delayOperator3->setDelayTimeUnit(Util::TimeUnit::minute);
	Release* releaseOperator3 = plugins->newInstance<Release>(model);
	releaseOperator3->getReleaseRequests()->insert(new SeizableItem(operator3, "1"));

	enterStation3->getConnectionManager()->insert(seizeOperator3);
	seizeOperator3->getConnectionManager()->insert(delayOperator3);
	delayOperator3->getConnectionManager()->insert(releaseOperator3);

	Decide* decide1 = plugins->newInstance<Decide>(model);
	decide1->getConditions()->insert("UNIF(0,1) < 0.11");

	releaseOperator3->getConnectionManager()->insert(decide1);

	// 10% de chance de seguir este caminho
	Assign* assign2 = plugins->newInstance<Assign>(model);
	assign2->getAssignments()->insert(new Assignment("v_WIP", "v_WIP-1"));
	assign2->getAssignments()->insert(new Assignment("countProducts", "countProducts+1"));

	decide1->getConnectionManager()->insert(assign2);

	Record* record2 = plugins->newInstance<Record>(model, "Record_Cycle_Time1");
	record2->setExpression("TNOW - a_time");

	assign2->getConnectionManager()->insert(record2);

	Dispose* dispose1 = plugins->newInstance<Dispose>(model);

	record2->getConnectionManager()->insert(dispose1);

	//90% de chance de seguir este caminho
	Station* station4 = plugins->newInstance<Station>(model, "Estacao_4");
	Enter* enterStation4 = plugins->newInstance<Enter>(model);
	enterStation4->setStation(station4);
	Route* route4 = plugins->newInstance<Route>(model);
	route4->setStation(station4);
	route4->setRouteTimeExpression("40");
	route4->setRouteTimeTimeUnit(Util::TimeUnit::second);

	decide1->getConnectionManager()->insert(route4);

	Resource* operator4 = plugins->newInstance<Resource>(model, "operador_4");
	operator4->setCapacity(1);
	Queue* queueOperator4 = plugins->newInstance<Queue>(model, "Fila_Operador4");
	Seize* seizeOperator4 = plugins->newInstance<Seize>(model);
	seizeOperator4->setQueue(queueOperator4);
	seizeOperator4->getSeizeRequests()->insert(new SeizableItem(operator4, "1"));
	Delay* delayOperator4 = plugins->newInstance<Delay>(model);
	delayOperator4->setDelayExpression("NORM(3,1.5)");
	delayOperator4->setDelayTimeUnit(Util::TimeUnit::minute);
	Release* releaseOperator4 = plugins->newInstance<Release>(model);
	releaseOperator4->getReleaseRequests()->insert(new SeizableItem(operator4, "1"));

	enterStation4->getConnectionManager()->insert(seizeOperator4);
	seizeOperator4->getConnectionManager()->insert(delayOperator4);
	delayOperator4->getConnectionManager()->insert(releaseOperator4);

	Assign* assign3 = plugins->newInstance<Assign>(model);
	assign3->getAssignments()->insert(new Assignment("v_WIP", "v_WIP-1"));
	assign3->getAssignments()->insert(new Assignment("countProducts", "countProducts+1"));

	releaseOperator4->getConnectionManager()->insert(assign3);

	Record* record4 = plugins->newInstance<Record>(model, "Record_Cycle_Time2");
	record4->setExpression("TNOW - a_time");

	assign3->getConnectionManager()->insert(record4);

	Dispose* dispose2 = plugins->newInstance<Dispose>(model);

	record4->getConnectionManager()->insert(dispose2);

	// set options, save and simulate
	model->getSimulation()->setReplicationLength(60, Util::TimeUnit::hour);
	model->getSimulation()->setWarmUpPeriod(3);
	model->getSimulation()->setWarmUpPeriodTimeUnit(Util::TimeUnit::hour);
	model->getSimulation()->setNumberOfReplications(50);
	model->save("./models/Assembly_Line.gen");
	model->getSimulation()->start();
	delete genesys;
	return 0;
};

