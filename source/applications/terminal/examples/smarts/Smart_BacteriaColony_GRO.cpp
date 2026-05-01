/*
 * File:   Smart_BacteriaColony_GRO.cpp
 * Author: GRO
 *
 * Created on 1 de Maio de 2026
 */

#include "Smart_BacteriaColony_GRO.h"

#include "kernel/simulator/EntityType.h"
#include "kernel/simulator/ModelSimulation.h"
#include "kernel/simulator/Simulator.h"
#include "plugins/components/BiochemicalSimulation/BacteriaColony.h"
#include "plugins/components/DiscreteProcessing/Create.h"
#include "plugins/components/DiscreteProcessing/Dispose.h"
#include "plugins/data/BiochemicalSimulation/BacteriaSignalGrid.h"
#include "plugins/data/BiochemicalSimulation/GroProgram.h"
#include "../../../TraitsApp.h"

#include <iostream>

Smart_BacteriaColony_GRO::Smart_BacteriaColony_GRO() {
}

int Smart_BacteriaColony_GRO::main(int argc, char** argv) {
	(void)argc;
	(void)argv;

	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());

	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");

	Model* model = genesys->getModelManager()->newModel();

	EntityType* carrier = plugins->newInstance<EntityType>(model, "BacteriaCarrier");
	Create* create = plugins->newInstance<Create>(model, "Create_BacteriaColony_GRO");
	create->setEntityType(carrier);
	create->setFirstCreation(0.0);
	create->setTimeBetweenCreationsExpression("1.0", Util::TimeUnit::second);
	create->setEntitiesPerCreation(1);
	create->setMaxCreations(10);

	GroProgram* program = plugins->newInstance<GroProgram>(model, "GroProgram_BacteriaColony_GRO");
	// The program stays compact on purpose: motion, growth, signal release and GFP
	// are all visible in the GUI with a short replication.
program->setSourceCode(R"(
include gro

program main() := {
  if ( seeded == 0 ) {
    reset(),
    set ( "dt", 0.1 ),
    set ( "ecoli_growth_rate", 0.18 ),
    seeded := 1,
    ecoli ( [ x := 2, y := 2 ], program colony() ),
    ecoli ( [ x := 5, y := 2 ], program colony() ),
    ecoli ( [ x := 2, y := 5 ], program colony() ),
    ecoli ( [ x := 5, y := 5 ], program colony() )
  }
};

program colony() := {
  speed := 0.12 + 0.05 * local_signal + 0.01 * bacterium_generation;
  direction := direction + 0.26 + 0.03 * bacterium_generation;
  gfp := 18 + 28 * local_signal + 2 * volume;
  emit_signal ( 0.55 + 0.15 * local_signal );
}
)");

	BacteriaSignalGrid* signalGrid = plugins->newInstance<BacteriaSignalGrid>(model, "SignalGrid_BacteriaColony_GRO");
	signalGrid->setWidth(16);
	signalGrid->setHeight(16);
	signalGrid->setInitialSignal(0.0);
	signalGrid->setDiffusionRate(0.22);
	signalGrid->setDecayRate(0.04);

	BacteriaColony* colony = plugins->newInstance<BacteriaColony>(model, "BacteriaColony_BacteriaColony_GRO");
	colony->setGroProgram(program);
	colony->setSignalGrid(signalGrid);
	colony->setSimulationStep(0.1);
	colony->setNumSteps(10);
	colony->setInitialPopulation(4);
	colony->setColonyTimeUnit(Util::TimeUnit::second);
	colony->setGridWidth(16);
	colony->setGridHeight(16);

	Dispose* dispose = plugins->newInstance<Dispose>(model, "Dispose_BacteriaColony_GRO");

	create->connectTo(colony);
	colony->connectTo(dispose);

	model->getSimulation()->setReplicationLength(12.0, Util::TimeUnit::second);
	model->getSimulation()->setNumberOfReplications(1);
	model->getSimulation()->setTerminatingCondition("");

	model->save("./models/Smart_BacteriaColony_GRO.gen");
	model->getSimulation()->start();

	std::cout << "Smart_BacteriaColony_GRO: colonyTime=" << colony->getColonyTime()
	          << ", population=" << colony->getPopulationSize()
	          << ", bacteria=" << colony->getInternalBacteriaCount()
	          << ", signal(2,2)=" << colony->getSignalValueAt(2, 2)
	          << std::endl;

	delete genesys;
	return 0;
}
