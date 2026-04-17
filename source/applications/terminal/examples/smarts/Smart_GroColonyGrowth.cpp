/*
 * File:   Smart_GroColonyGrowth.cpp
 * Author: GRO
 *
 * Created on 17 de Abril de 2026
 */

#include "Smart_GroColonyGrowth.h"

#include "kernel/simulator/Simulator.h"
#include "plugins/components/BiologicalModeling/BacteriaColony.h"
#include "plugins/components/DiscreteProcessing/Create.h"
#include "plugins/components/DiscreteProcessing/Dispose.h"
#include "plugins/data/BiologicalModeling/GroProgram.h"
#include "../../../TraitsApp.h"

#include <iostream>

Smart_GroColonyGrowth::Smart_GroColonyGrowth() {
}

int Smart_GroColonyGrowth::main(int argc, char** argv) {
	(void)argc;
	(void)argv;

	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();

	GroProgram* program = plugins->newInstance<GroProgram>(model, "GroProgram_Growth");
	program->setSourceCode("program colony() { tick(); grow(2); }");

	Create* create = plugins->newInstance<Create>(model, "Create_GrowthPulse");
	create->setFirstCreation(0.0);
	create->setTimeBetweenCreationsExpression("1.0", Util::TimeUnit::second);
	create->setMaxCreations(3);

	BacteriaColony* colony = plugins->newInstance<BacteriaColony>(model, "BacteriaColony_Growth");
	colony->setGroProgram(program);
	colony->setSimulationStep(0.5);
	colony->setInitialColonyTime(0.0);
	colony->setInitialPopulation(2);
	colony->setGridWidth(4);
	colony->setGridHeight(2);

	Dispose* dispose = plugins->newInstance<Dispose>(model, "Dispose_GrowthPulse");

	create->connectTo(colony);
	colony->connectTo(dispose);

	model->getSimulation()->setReplicationLength(3.0, Util::TimeUnit::second);
	model->getSimulation()->setTerminatingCondition("");
	model->save("./models/Smart_GroColonyGrowth.gen");
	model->getSimulation()->start();

	std::cout << "Smart_GroColonyGrowth: colonyTime=" << colony->getColonyTime()
	          << ", population=" << colony->getPopulationSize()
	          << ", bacteria=" << colony->getInternalBacteriaCount()
	          << std::endl;

	delete genesys;
	return 0;
};
