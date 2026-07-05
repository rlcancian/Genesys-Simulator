/* 
 * File:   Smart_CellularAutomata1.cpp
 * Author: rlcancian
 * 
 * Created on 8 de fevereiro de 2022, 15:16
 */

#include "Smart_CellularAutomata1.h"

// you have to included need libs

// GEnSyS Simulator
#include "kernel/simulator/Simulator.h"

// Model Components
#include "plugins/components/Logic/Create.h"
#include "plugins/components/ModalModel/CellularAutomataComp.h"
#include "plugins/components/Logic/Dispose.h"

Smart_CellularAutomata1::Smart_CellularAutomata1() {
}

/**
 * This is the main function of the application.
 * It instanciates the simulator, builds a simulation model and then simulate that model.
 */
int Smart_CellularAutomata1::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();
	// create model
	Create* create1 = plugins->newInstance<Create>(model);
	CellularAutomataComp* ca = plugins->newInstance<CellularAutomataComp>(model);
	ca->setCellularAutomataType(CellularAutomataComp::CellularAutomataType::CLASSIC);
	ca->setLatticeType(CellularAutomataComp::LatticeType::RETICULAR);
	ca->getlattice()->setDimensions({100,100});
	ca->setNeighboorhoodType(CellularAutomataComp::NeighboorhoodType::MOORE);
	ca->getNeighboorhood()->setRadius(2);
	ca->setBoundaryType(CellularAutomataComp::BoundaryType::FIXED);
	ca->setStateSetType(CellularAutomataComp::StateSetType::ENUMERATED);
	ca->setLocalRuleType(CellularAutomataComp::LocalRuleType::SAND_ROCK);
	Dispose* dispose1 = plugins->newInstance<Dispose>(model);
	// connect model components to create a "workflow"
	create1->getConnectionManager()->insert(ca);
	ca->getConnectionManager()->insert(dispose1);
	// set options, save and simulate
	model->getSimulation()->setReplicationLength(60, Util::TimeUnit::second);
	model->getSimulation()->setTerminatingCondition("count(Dispose_1.CountNumberIn)>30");
	model->save("./models/Smart_CellularAutomata.gen");
	model->getSimulation()->start();
	delete genesys;
	return 0;
};
