/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Smart_CellularAutomata.cpp
 * Author: rlcancian
 * 
 * Created on 13/11/2023
 */

#include "Smart_CellularAutomata.h"

// you have to included need libs

// GEnSyS Simulator
#include "kernel/simulator/Simulator.h"

// Model Components
#include "plugins/components/DiscreteProcessing/Create.h"
#include "plugins/components/Logic/CellularAutomataComp.h"
#include "plugins/components/DiscreteProcessing/Dispose.h"
#include "../../../TraitsApp.h"

Smart_CellularAutomata::Smart_CellularAutomata() {
}

/**
 * This is the main function of the application. 
 * It instanciates the simulator, builds a simulation model and then simulate that model.
 */
int Smart_CellularAutomata::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	genesys->getTracer()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTracer());
	PluginManager* plugins = genesys->getPlugins();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModels()->newModel();
	// create model
	Create* create1 = plugins->newInstance<Create>(model);
	CellularAutomataComp* ca = plugins->newInstance<CellularAutomataComp>(model);
	ca->setCellularAutomataType(CellularAutomataComp::CellularAutomataType::CLASSIC);
	ca->setLatticeType(CellularAutomataComp::LatticeType::RETICULAR);
	ca->getlattice()->setDimensions({10,10});
	ca->setNeighboorhoodType(CellularAutomataComp::NeighboorhoodType::VONNEUMANN);
	ca->getNeighboorhood()->setRadius(1);
	ca->setBoundaryType(CellularAutomataComp::BoundaryType::FIXED);
	ca->setStateSetType(CellularAutomataComp::StateSetType::ENUMERATED);
	Dispose* dispose1 = plugins->newInstance<Dispose>(model);
	// connect model components to create a "workflow"
	create1->getConnections()->insert(ca);
	ca->getConnections()->insert(dispose1);
	// set options, save and simulate
	model->getSimulation()->setReplicationLength(60, Util::TimeUnit::second);
	model->getSimulation()->setTerminatingCondition("count(Dispose_1.CountNumberIn)>30");
	model->save("./models/Smart_CellularAutomata.gen");
	model->getSimulation()->start();
	delete genesys;
	return 0;
};

