/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Smart_RouteStation.cpp
 * Author: rlcancian
 * 
 * Created on 3 de Setembro de 2019, 18:34
 */

#include "Smart_RouteStation.h"

// you have to included need libs

// GEnSyS Simulator
#include "../../../../kernel/simulator/Simulator.h"

// Model Components
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/Route.h"
#include "../../../../plugins/components/Enter.h"
#include "../../../../plugins/components/Delay.h"
#include "../../../../plugins/components/Dispose.h"
#include "../../../TraitsApp.h"

Smart_RouteStation::Smart_RouteStation() {
}

/**
 * This is the main function of the application. 
 * It instanciates the simulator, builds a simulation model and then simulate that model.
 */
int Smart_RouteStation::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();
	// create model
	Create* create1 = plugins->newInstance<Create>(model);
	create1->setTimeBetweenCreationsExpression("4.0");
	Route* route1 = plugins->newInstance<Route>(model);
	route1->setStationName("station1");
	route1->setRouteTimeExpression("0.1");
	Enter* enter1 = plugins->newInstance<Enter>(model);
	enter1->setStationName("station1");
	Delay* delay1 = plugins->newInstance<Delay>(model);
	Route* route2 = plugins->newInstance<Route>(model);
	route2->setStationName("station2");
	route2->setRouteTimeExpression("0.2");
	Enter* enter2 = plugins->newInstance<Enter>(model);
	enter2->setStationName("station2");
	Delay* delay2 = plugins->newInstance<Delay>(model);
	Route* route3 = plugins->newInstance<Route>(model);
	route3->setStationName("station3");
	route3->setRouteTimeExpression("0.3");
	Enter* enter3 = plugins->newInstance<Enter>(model);
	enter3->setStationName("station3");
	Dispose* dispose1 = plugins->newInstance<Dispose>(model);
	// connect model components to create a "workflow"
	create1->getConnectionManager()->insert(route1);
	enter1->getConnectionManager()->insert(delay1);
	delay1->getConnectionManager()->insert(route2);
	enter2->getConnectionManager()->insert(delay2);
	delay2->getConnectionManager()->insert(route3);
	enter3->getConnectionManager()->insert(dispose1);
	// set options, save and simulate
	model->getSimulation()->setReplicationLength(20);
	model->save("./models/Smart_RouteStation.gen");
	/*
	do {
		model->getSimulation()->step();
		std::cin.ignore(std::numeric_limits <std::streamsize> ::max(), '\n');
	} while (model->getSimulation()->isPaused());
	 */
	model->getSimulation()->start();
	delete genesys;
	return 0;
};

