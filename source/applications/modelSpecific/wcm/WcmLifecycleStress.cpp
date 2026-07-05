#include "WcmLifecycleStress.h"

#include <iostream>

#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/model/Model.h"

#include "plugins/components/Logic/Create.h"
#include "plugins/components/Logic/Dispose.h"
#include "plugins/components/WholeCellModeling/CellCycleCheckpointComponent.h"
#include "plugins/components/WholeCellModeling/CellFateDecisionComponent.h"
#include "plugins/data/WholeCellModeling/WholeCellState.h"

WcmLifecycleStress::WcmLifecycleStress() {}

int WcmLifecycleStress::main(int argc, char** argv) {
	(void)argc;
	(void)argv;

	Simulator* genesys = new Simulator();
	this->setDefaultTraceHandlers(genesys->getTraceManager());
	genesys->getPluginManager()->autoInsertPlugins();

	Model* model = genesys->getModelManager()->newModel();
	PluginManager* plugins = genesys->getPluginManager();

	model->getInfos()->setName("Whole-Cell Lifecycle Stress");
	model->getInfos()->setDescription(
		"Minimal stress example that demonstrates whole-cell starvation and death routing.");

	WholeCellState* state = plugins->newInstance<WholeCellState>(model, "StressState");
	state->setCellMass(1.0e-15);
	state->setCellVolume(1.0e-3);
	state->setMetaboliteAmount("ATP", 0.0);
	state->setLifecyclePhase("newborn");

	Create* clock = plugins->newInstance<Create>(model, "StressClock");
	clock->setTimeBetweenCreationsExpression("60", Util::TimeUnit::second);
	clock->setMaxCreations(3);

	CellCycleCheckpointComponent* checkpoint = plugins->newInstance<CellCycleCheckpointComponent>(model, "StressCheckpoint");
	checkpoint->setWholeCellState(state);
	checkpoint->setDeltaT(60.0);
	checkpoint->setEnergyMetaboliteKey("ATP");
	checkpoint->setStarvationAtpThreshold(0.25);
	checkpoint->setLethalStarvationSteps(2u);
	checkpoint->setAdvanceWholeCellClock(true);

	CellFateDecisionComponent* fate = plugins->newInstance<CellFateDecisionComponent>(model, "StressFate");
	fate->setWholeCellState(state);

	Dispose* viableSink = plugins->newInstance<Dispose>(model, "StressViableSink");
	Dispose* divisionSink = plugins->newInstance<Dispose>(model, "StressDivisionSink");
	Dispose* starvedSink = plugins->newInstance<Dispose>(model, "StressStarvedSink");
	Dispose* deadSink = plugins->newInstance<Dispose>(model, "StressDeadSink");

	clock->connectTo(checkpoint);
	checkpoint->connectTo(fate);
	fate->connectTo(viableSink);
	fate->connectTo(divisionSink);
	fate->connectTo(starvedSink);
	fate->connectTo(deadSink);

	ModelSimulation* sim = model->getSimulation();
	sim->setNumberOfReplications(1);
	sim->setReplicationLength(180.0, Util::TimeUnit::second);
	sim->setReplicationReportBaseTimeUnit(Util::TimeUnit::second);
	model->getTracer()->setTraceLevel(TraceManager::Level::L2_results);

	model->save("./models/WcmLifecycleStress.gen");
	sim->start();

	std::cout << "whole_cell_lifecycle_stress"
	          << " time=" << state->getCurrentTime()
	          << " phase=" << state->getLifecyclePhase()
	          << " generation=" << state->getGenerationCount()
	          << " viable=" << (state->isViable() ? "true" : "false")
	          << " last_port=" << fate->getLastRoutedPort()
	          << std::endl;

	delete genesys;
	return 0;
}
