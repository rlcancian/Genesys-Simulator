#include "WcmPathwayStressWholeCellMvp.h"

#include <iostream>

#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/model/Model.h"

#include "plugins/components/BiochemicalSimulation/MetabolicFluxBalance.h"
#include "plugins/components/Logic/Create.h"
#include "plugins/components/Logic/Dispose.h"
#include "plugins/components/WholeCellModeling/CellCycleCheckpointComponent.h"
#include "plugins/components/WholeCellModeling/CellFateDecisionComponent.h"
#include "plugins/components/WholeCellModeling/MetabolicStateProjectionComponent.h"
#include "plugins/components/WholeCellModeling/PathwayStressResponseComponent.h"

#include "plugins/data/BiochemicalSimulation/BioSpecies.h"
#include "plugins/data/BiochemicalSimulation/MetabolicNetwork.h"
#include "plugins/data/BiochemicalSimulation/MetabolicReaction.h"
#include "plugins/data/WholeCellModeling/WholeCellState.h"

WcmPathwayStressWholeCellMvp::WcmPathwayStressWholeCellMvp() {}

int WcmPathwayStressWholeCellMvp::main(int argc, char** argv) {
	(void) argc;
	(void) argv;

	Simulator* genesys = new Simulator();
	this->setDefaultTraceHandlers(genesys->getTraceManager());
	genesys->getPluginManager()->autoInsertPlugins();

	Model* model = genesys->getModelManager()->newModel();
	PluginManager* plugins = genesys->getPluginManager();

	model->getInfos()->setName("Pathway-Stress Whole-Cell MVP");
	model->getInfos()->setDescription(
		"Whole-cell stress example using FBA, pathway projection, checkpointing, "
		"and sustained pathway-collapse response.");

	BioSpecies* atpIntermediate = plugins->newInstance<BioSpecies>(model, "StressATP_i");
	atpIntermediate->setInitialAmount(0.0);
	atpIntermediate->setAmount(0.0);

	MetabolicReaction* uptake = plugins->newInstance<MetabolicReaction>(model, "StressATPUptake");
	uptake->addProduct("StressATP_i", 1.0);
	uptake->setLowerBound(0.0);
	uptake->setUpperBound(4.0);

	MetabolicReaction* maintenance = plugins->newInstance<MetabolicReaction>(model, "StressATPMaintenance");
	maintenance->addReactant("StressATP_i", 1.0);
	maintenance->setLowerBound(1.0);
	maintenance->setUpperBound(1.0);

	MetabolicReaction* biomass = plugins->newInstance<MetabolicReaction>(model, "StressBiomassFlux");
	biomass->addReactant("StressATP_i", 1.0);
	biomass->setLowerBound(0.0);
	biomass->setUpperBound(100.0);

	MetabolicNetwork* network = plugins->newInstance<MetabolicNetwork>(model, "StressNetwork");
	network->addReaction("StressATPUptake");
	network->addReaction("StressATPMaintenance");
	network->addReaction("StressBiomassFlux");
	network->setObjectiveReactionName("StressBiomassFlux");
	network->setObjectiveSense("Maximize");
	network->setCompartment("cytosol");

	WholeCellState* state = plugins->newInstance<WholeCellState>(model, "StressCellState");
	state->setMetaboliteAmount("ATP", 0.0);
	state->setLifecyclePhase("newborn");
	state->setViable(true);

	Create* clock = plugins->newInstance<Create>(model, "StressClock");
	clock->setTimeBetweenCreationsExpression("60", Util::TimeUnit::second);
	clock->setMaxCreations(4);

	MetabolicFluxBalance* fluxBalance = plugins->newInstance<MetabolicFluxBalance>(model, "StressFluxBalance");
	fluxBalance->setMetabolicNetwork(network);

	MetabolicStateProjectionComponent* projection = plugins->newInstance<MetabolicStateProjectionComponent>(model, "StressProjection");
	projection->setWholeCellState(state);
	projection->setFluxBalanceComponent(fluxBalance);
	projection->setObjectiveAsPathwayActivity("biomass_objective");
	projection->addMetaboliteProjection("StressATPUptake", "ATP", 0.5);
	projection->addPathwayProjection("StressBiomassFlux", "biomass_flux");

	CellCycleCheckpointComponent* checkpoint = plugins->newInstance<CellCycleCheckpointComponent>(model, "StressCheckpoint");
	checkpoint->setWholeCellState(state);
	checkpoint->setDeltaT(60.0);
	checkpoint->setEnergyMetaboliteKey("ATP");
	checkpoint->setStarvationAtpThreshold(0.25);
	checkpoint->setAdvanceWholeCellClock(true);

	PathwayStressResponseComponent* stress = plugins->newInstance<PathwayStressResponseComponent>(model, "StressResponse");
	stress->setWholeCellState(state);
	stress->setMonitoredPathwayKey("biomass_objective");
	stress->setStressThreshold(4.0);
	stress->setArrestAfterSteps(2u);
	stress->setDeathAfterSteps(4u);
	stress->setArrestPhase("arrested");
	stress->setDeadPhase("dead");
	stress->setRecoveryPhase("growth");

	CellFateDecisionComponent* fate = plugins->newInstance<CellFateDecisionComponent>(model, "StressFate");
	fate->setWholeCellState(state);

	Dispose* steadySink = plugins->newInstance<Dispose>(model, "StressSteadySink");
	Dispose* divisionSink = plugins->newInstance<Dispose>(model, "StressDivisionSink");
	Dispose* stressSink = plugins->newInstance<Dispose>(model, "StressArrestSink");
	Dispose* deadSink = plugins->newInstance<Dispose>(model, "StressDeadSink");

	clock->connectTo(fluxBalance);
	fluxBalance->connectTo(projection);
	projection->connectTo(checkpoint);
	checkpoint->connectTo(stress);
	stress->connectTo(fate);
	fate->connectTo(steadySink);
	fate->connectTo(divisionSink);
	fate->connectTo(stressSink);
	fate->connectTo(deadSink);

	ModelSimulation* sim = model->getSimulation();
	sim->setNumberOfReplications(1);
	sim->setReplicationLength(240.0, Util::TimeUnit::second);
	sim->setReplicationReportBaseTimeUnit(Util::TimeUnit::second);
	model->getTracer()->setTraceLevel(TraceManager::Level::L2_results);

	model->save("./models/WcmPathwayStressWholeCellMvp.gen");
	sim->start();

	std::cout << "whole_cell_pathway_stress_mvp"
	          << " objective=" << fluxBalance->getLastObjectiveValue()
	          << " ATP=" << state->getMetaboliteAmount("ATP")
	          << " biomass_objective=" << state->getPathwayActivity("biomass_objective")
	          << " biomass_flux=" << state->getPathwayActivity("biomass_flux")
	          << " time=" << state->getCurrentTime()
	          << " phase=" << state->getLifecyclePhase()
	          << " viable=" << (state->isViable() ? "true" : "false")
	          << " stress_streak=" << stress->getStressStreak()
	          << " last_port=" << fate->getLastRoutedPort()
	          << std::endl;

	delete genesys;
	return 0;
}
