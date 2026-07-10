#include "WcmFluxBalanceWholeCellMvp.h"

#include <iostream>

#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/model/Model.h"

#include "plugins/components/BiochemicalSimulation/MetabolicFluxBalance.h"
#include "plugins/components/Logic/Create.h"
#include "plugins/components/Logic/Dispose.h"
#include "plugins/components/WholeCellModeling/CellCycleCheckpointComponent.h"
#include "plugins/components/WholeCellModeling/CellDivisionEvent.h"
#include "plugins/components/WholeCellModeling/CellFateDecisionComponent.h"
#include "plugins/components/WholeCellModeling/CellGrowthComponent.h"
#include "plugins/components/WholeCellModeling/MetabolicStateProjectionComponent.h"
#include "plugins/components/WholeCellModeling/ResourceAllocationComponent.h"
#include "plugins/components/WholeCellModeling/StochasticTranscription.h"
#include "plugins/components/WholeCellModeling/StochasticTranslation.h"

#include "plugins/data/BiochemicalSimulation/BioSpecies.h"
#include "plugins/data/BiochemicalSimulation/MetabolicNetwork.h"
#include "plugins/data/BiochemicalSimulation/MetabolicReaction.h"
#include "plugins/data/WholeCellModeling/WholeCellState.h"

WcmFluxBalanceWholeCellMvp::WcmFluxBalanceWholeCellMvp() {}

int WcmFluxBalanceWholeCellMvp::main(int argc, char** argv) {
	(void) argc;
	(void) argv;

	Simulator* genesys = new Simulator();
	this->setDefaultTraceHandlers(genesys->getTraceManager());
	genesys->getPluginManager()->autoInsertPlugins();

	Model* model = genesys->getModelManager()->newModel();
	PluginManager* plugins = genesys->getPluginManager();

	model->getInfos()->setName("Flux-Balance Whole-Cell MVP");
	model->getInfos()->setDescription(
		"Whole-cell MVP using MetabolicFluxBalance plus MetabolicStateProjectionComponent "
		"to drive expression, growth, lifecycle checkpointing, and cell division.");

	BioSpecies* atpIntermediate = plugins->newInstance<BioSpecies>(model, "FbaATP_i");
	atpIntermediate->setInitialAmount(0.0);
	atpIntermediate->setAmount(0.0);

	MetabolicReaction* uptake = plugins->newInstance<MetabolicReaction>(model, "FbaATPUptake");
	uptake->addProduct("FbaATP_i", 1.0);
	uptake->setLowerBound(0.0);
	uptake->setUpperBound(4.0);

	MetabolicReaction* maintenance = plugins->newInstance<MetabolicReaction>(model, "FbaATPMaintenance");
	maintenance->addReactant("FbaATP_i", 1.0);
	maintenance->setLowerBound(1.0);
	maintenance->setUpperBound(1.0);

	MetabolicReaction* biomass = plugins->newInstance<MetabolicReaction>(model, "FbaBiomassFlux");
	biomass->addReactant("FbaATP_i", 1.0);
	biomass->setLowerBound(0.0);
	biomass->setUpperBound(100.0);

	MetabolicNetwork* network = plugins->newInstance<MetabolicNetwork>(model, "FbaWholeCellNetwork");
	network->addReaction("FbaATPUptake");
	network->addReaction("FbaATPMaintenance");
	network->addReaction("FbaBiomassFlux");
	network->setObjectiveReactionName("FbaBiomassFlux");
	network->setObjectiveSense("Maximize");
	network->setCompartment("cytosol");

	WholeCellState* state = plugins->newInstance<WholeCellState>(model, "CellState");
	state->setCellMass(1.0e-15);
	state->setCellVolume(1.0e-3);
	state->setMoleculeCount("mRNA_geneA", 0);
	state->setMoleculeCount("mRNA_geneB", 0);
	state->setMoleculeCount("prot_geneA", 0);
	state->setMoleculeCount("prot_geneB", 0);
	state->setMoleculeCount("RNAP_free", 4);
	state->setMoleculeCount("ribosome_free", 6);
	state->setMetaboliteAmount("ATP", 0.0);
	state->setLifecyclePhase("newborn");

	Create* clock = plugins->newInstance<Create>(model, "CellClock");
	clock->setTimeBetweenCreationsExpression("60", Util::TimeUnit::second);
	clock->setMaxCreations(8);

	MetabolicFluxBalance* fluxBalance = plugins->newInstance<MetabolicFluxBalance>(model, "FluxBalance");
	fluxBalance->setMetabolicNetwork(network);

	MetabolicStateProjectionComponent* projection = plugins->newInstance<MetabolicStateProjectionComponent>(model, "FluxProjection");
	projection->setWholeCellState(state);
	projection->setFluxBalanceComponent(fluxBalance);
	projection->setObjectiveAsPathwayActivity("biomass_objective");
	projection->addMetaboliteProjection("FbaATPUptake", "ATP", 0.25, 0.0,
		MetabolicStateProjectionComponent::ProjectionUpdateMode::Accumulate);
	projection->addCompartmentMetaboliteProjection("FbaATPUptake", "cytosol", "ATP_c", 0.5, 0.0,
		MetabolicStateProjectionComponent::ProjectionUpdateMode::Turnover, 0.25);
	projection->addPathwayProjection("FbaBiomassFlux", "biomass_flux");

	ResourceAllocationComponent* allocation = plugins->newInstance<ResourceAllocationComponent>(model, "ResourceAllocation");
	allocation->setWholeCellState(state);
	allocation->setRnapCountKey("RNAP_free");
	allocation->setRibosomeCountKey("ribosome_free");
	allocation->setMRNASpeciesPrefix("mRNA_");
	allocation->setProteinSpeciesPrefix("prot_");

	StochasticTranscription* transcription = plugins->newInstance<StochasticTranscription>(model, "Transcription");
	transcription->setWholeCellState(state);
	transcription->setElongationRate(50.0);
	transcription->setMeanGeneLength(900.0);
	transcription->setBindingProbability(0.30);
	transcription->setTimeWindow(60.0);
	transcription->setMRNASpeciesPrefix("mRNA_");
	transcription->setRnapCountKey("RNAP_free");
	transcription->setRandomSeed(401u);

	StochasticTranslation* translation = plugins->newInstance<StochasticTranslation>(model, "Translation");
	translation->setWholeCellState(state);
	translation->setElongationRate(16.0);
	translation->setMeanProteinLength(300.0);
	translation->setTimeWindow(60.0);
	translation->setMRNASpeciesPrefix("mRNA_");
	translation->setProteinSpeciesPrefix("prot_");
	translation->setRibosomeCountKey("ribosome_free");
	translation->setRandomSeed(402u);

	CellGrowthComponent* growth = plugins->newInstance<CellGrowthComponent>(model, "Growth");
	growth->setWholeCellState(state);
	growth->setGrowthRate(0.0025);
	growth->setDeltaT(60.0);
	growth->setDensity(1000.0);
	growth->setEnergyMetaboliteKey("ATP");
	growth->setEnergyHalfSaturation(1.0);

	CellCycleCheckpointComponent* checkpoint = plugins->newInstance<CellCycleCheckpointComponent>(model, "LifecycleCheckpoint");
	checkpoint->setWholeCellState(state);
	checkpoint->setDeltaT(60.0);
	checkpoint->setEnergyMetaboliteKey("ATP");
	checkpoint->setStarvationAtpThreshold(0.25);
	checkpoint->setCompartmentEnergyRegion("cytosol");
	checkpoint->setCompartmentEnergyMetaboliteKey("ATP_c");
	checkpoint->setCompartmentStarvationThreshold(0.5);
	checkpoint->setCriticalPathwayActivityKey("biomass_objective");
	checkpoint->setCriticalPathwayActivityThreshold(2.5);
	checkpoint->setDivisionMassThreshold(1.8e-15);
	checkpoint->setAdvanceWholeCellClock(true);

	CellFateDecisionComponent* fate = plugins->newInstance<CellFateDecisionComponent>(model, "CellFate");
	fate->setWholeCellState(state);

	CellDivisionEvent* division = plugins->newInstance<CellDivisionEvent>(model, "Division");
	division->setWholeCellState(state);
	division->setDivisionMassThreshold(1.8e-15);
	division->setFtsZThreshold(0.0);
	division->setRandomSeed(403u);

	Dispose* steadySink = plugins->newInstance<Dispose>(model, "SteadySink");
	Dispose* divisionSink = plugins->newInstance<Dispose>(model, "DivisionSink");
	Dispose* starvedSink = plugins->newInstance<Dispose>(model, "StarvedSink");
	Dispose* deadSink = plugins->newInstance<Dispose>(model, "DeadSink");

	clock->connectTo(fluxBalance);
	fluxBalance->connectTo(projection);
	projection->connectTo(allocation);
	allocation->connectTo(transcription);
	transcription->connectTo(translation);
	translation->connectTo(growth);
	growth->connectTo(checkpoint);
	checkpoint->connectTo(fate);
	fate->connectTo(steadySink);
	fate->connectTo(division);
	fate->connectTo(starvedSink);
	fate->connectTo(deadSink);
	division->connectTo(steadySink);
	division->connectTo(divisionSink);

	ModelSimulation* sim = model->getSimulation();
	sim->setNumberOfReplications(1);
	sim->setReplicationLength(480.0, Util::TimeUnit::second);
	sim->setReplicationReportBaseTimeUnit(Util::TimeUnit::second);
	model->getTracer()->setTraceLevel(TraceManager::Level::L2_results);

	model->save("./models/WcmFluxBalanceWholeCellMvp.gen");
	sim->start();

	std::cout << "whole_cell_fba_mvp"
	          << " objective=" << fluxBalance->getLastObjectiveValue()
	          << " ATP=" << state->getMetaboliteAmount("ATP")
	          << " ATP_c=" << state->getCompartmentMetaboliteAmount("cytosol", "ATP_c")
	          << " biomass_flux=" << state->getPathwayActivity("biomass_flux")
	          << " mass=" << state->getCellMass()
	          << " volume=" << state->getCellVolume()
	          << " time=" << state->getCurrentTime()
	          << " phase=" << state->getLifecyclePhase()
	          << " generation=" << state->getGenerationCount()
	          << " viable=" << (state->isViable() ? "true" : "false")
	          << " divisions=" << division->getDivisionCount()
	          << " mRNA_total=" << (state->getMoleculeCount("mRNA_geneA") + state->getMoleculeCount("mRNA_geneB"))
	          << " protein_total=" << (state->getMoleculeCount("prot_geneA") + state->getMoleculeCount("prot_geneB"))
	          << std::endl;

	delete genesys;
	return 0;
}
