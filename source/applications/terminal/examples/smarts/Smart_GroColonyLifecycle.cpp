/*
 * File:   Smart_GroColonyLifecycle.cpp
 * Author: GRO
 *
 * Created on 17 de Abril de 2026
 */

#include "Smart_GroColonyLifecycle.h"

#include "kernel/simulator/Simulator.h"
#include "plugins/components/BiologicalModeling/BacteriaColony.h"
#include "plugins/components/DiscreteProcessing/Create.h"
#include "plugins/components/DiscreteProcessing/Dispose.h"
#include "plugins/data/BiologicalModeling/GroProgram.h"
#include "../../../TraitsApp.h"

#include <iostream>

Smart_GroColonyLifecycle::Smart_GroColonyLifecycle() {
}

int Smart_GroColonyLifecycle::main(int argc, char** argv) {
	(void)argc;
	(void)argv;

	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();

	GroProgram* program = plugins->newInstance<GroProgram>(model, "GroProgram_Lifecycle");
	program->setSourceCode("program colony() { tick(); divide(); die(1); }");

	Create* create = plugins->newInstance<Create>(model, "Create_LifecyclePulse");
	create->setFirstCreation(0.0);
	create->setTimeBetweenCreationsExpression("1.0", Util::TimeUnit::second);
	create->setMaxCreations(2);

	BacteriaColony* colony = plugins->newInstance<BacteriaColony>(model, "BacteriaColony_Lifecycle");
	colony->setGroProgram(program);
	colony->setSimulationStep(1.0);
	colony->setInitialColonyTime(5.0);
	colony->setInitialPopulation(3);
	colony->setGridWidth(3);
	colony->setGridHeight(3);

	Dispose* dispose = plugins->newInstance<Dispose>(model, "Dispose_LifecyclePulse");

	create->connectTo(colony);
	colony->connectTo(dispose);

	model->getSimulation()->setReplicationLength(2.0, Util::TimeUnit::second);
	model->getSimulation()->setTerminatingCondition("");
	model->save("./models/Smart_GroColonyLifecycle.gen");
	model->getSimulation()->start();

	std::cout << "Smart_GroColonyLifecycle: colonyTime=" << colony->getColonyTime()
	          << ", population=" << colony->getPopulationSize()
	          << ", firstDivisionCount=" << colony->getBacteriumState(0).divisionCount
	          << ", youngestGeneration=" << colony->getBacteriumState(colony->getInternalBacteriaCount() - 1).generation
	          << std::endl;

	delete genesys;
	return 0;
};
