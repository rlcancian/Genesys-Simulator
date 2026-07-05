#include "WcmWholeCellMvp.h"

#include <iostream>
#include <string>

#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/model/Model.h"

#include "plugins/components/BiochemicalSimulation/BioSimulate.h"
#include "plugins/components/Logic/Create.h"
#include "plugins/components/Logic/Dispose.h"
#include "plugins/components/WholeCellModeling/BioStateProjectionComponent.h"
#include "plugins/components/WholeCellModeling/CellCycleCheckpointComponent.h"
#include "plugins/components/WholeCellModeling/CellFateDecisionComponent.h"
#include "plugins/components/WholeCellModeling/CellDivisionEvent.h"
#include "plugins/components/WholeCellModeling/CellGrowthComponent.h"
#include "plugins/components/WholeCellModeling/ResourceAllocationComponent.h"
#include "plugins/components/WholeCellModeling/StochasticTranscription.h"
#include "plugins/components/WholeCellModeling/StochasticTranslation.h"

#include "plugins/data/BiochemicalSimulation/BioNetwork.h"
#include "plugins/data/BiochemicalSimulation/BioParameter.h"
#include "plugins/data/BiochemicalSimulation/BioReaction.h"
#include "plugins/data/BiochemicalSimulation/BioSpecies.h"
#include "plugins/data/WholeCellModeling/WholeCellState.h"

WcmWholeCellMvp::WcmWholeCellMvp() {}

int WcmWholeCellMvp::main(int argc, char** argv) {
	(void)argc;
	(void)argv;

	Simulator* genesys = new Simulator();
	this->setDefaultTraceHandlers(genesys->getTraceManager());
	genesys->getPluginManager()->autoInsertPlugins();

	Model* model = genesys->getModelManager()->newModel();
	PluginManager* plugins = genesys->getPluginManager();

	model->getInfos()->setName("Whole-Cell MVP");
	model->getInfos()->setDescription(
		"Minimal whole-cell MVP with deterministic metabolism, runtime state projection, "
		"stochastic gene expression, energy-gated growth, and mass-triggered division.");

	BioSpecies* nutrient = plugins->newInstance<BioSpecies>(model, "Nutrient");
	nutrient->setInitialAmount(200.0);
	nutrient->setAmount(200.0);

	BioSpecies* energy = plugins->newInstance<BioSpecies>(model, "EnergyPool");
	energy->setInitialAmount(0.0);
	energy->setAmount(0.0);

	BioParameter* uptakeRate = plugins->newInstance<BioParameter>(model, "kUptake");
	uptakeRate->setValue(0.03);

	BioReaction* uptake = plugins->newInstance<BioReaction>(model, "NutrientToEnergy");
	uptake->addReactant("Nutrient", 1.0);
	uptake->addProduct("EnergyPool", 1.0);
	uptake->setKineticLawExpression("kUptake * Nutrient");

	BioNetwork* metabolism = plugins->newInstance<BioNetwork>(model, "Metabolism");
	metabolism->addSpecies("Nutrient");
	metabolism->addSpecies("EnergyPool");
	metabolism->addReaction("NutrientToEnergy");
	metabolism->setStartTime(0.0);
	metabolism->setStopTime(60.0);
	metabolism->setStepSize(5.0);

	WholeCellState* state = plugins->newInstance<WholeCellState>(model, "CellState");
	state->setCellMass(1.0e-15);
	state->setCellVolume(1.0e-3);
	state->setMoleculeCount("mRNA_geneA", 0);
	state->setMoleculeCount("mRNA_geneB", 0);
	state->setMoleculeCount("prot_geneA", 0);
	state->setMoleculeCount("prot_geneB", 0);
	state->setMoleculeCount("RNAP_free", 0);
	state->setMoleculeCount("ribosome_free", 0);
	state->setMetaboliteAmount("ATP", 0.0);
	state->setLifecyclePhase("newborn");

	Create* clock = plugins->newInstance<Create>(model, "CellClock");
	clock->setTimeBetweenCreationsExpression("60", Util::TimeUnit::second);
	clock->setMaxCreations(8);

	BioSimulate* bioSimulate = plugins->newInstance<BioSimulate>(model, "BioSimulateMetabolism");
	bioSimulate->setBioNetwork(metabolism);
	bioSimulate->setUseNetworkTimeWindow(true);

	BioStateProjectionComponent* projection = plugins->newInstance<BioStateProjectionComponent>(model, "EnergyProjection");
	projection->setWholeCellState(state);
	projection->addMetaboliteProjection("EnergyPool", "ATP", 0.02);
	projection->addMoleculeProjection("EnergyPool", "RNAP_free", 0.05);
	projection->addMoleculeProjection("EnergyPool", "ribosome_free", 0.08);

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
	transcription->setBindingProbability(0.40);
	transcription->setTimeWindow(60.0);
	transcription->setMRNASpeciesPrefix("mRNA_");
	transcription->setRnapCountKey("RNAP_free");
	transcription->setRandomSeed(110u);

	StochasticTranslation* translation = plugins->newInstance<StochasticTranslation>(model, "Translation");
	translation->setWholeCellState(state);
	translation->setElongationRate(16.0);
	translation->setMeanProteinLength(300.0);
	translation->setTimeWindow(60.0);
	translation->setMRNASpeciesPrefix("mRNA_");
	translation->setProteinSpeciesPrefix("prot_");
	translation->setRibosomeCountKey("ribosome_free");
	translation->setRandomSeed(220u);

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
	checkpoint->setDivisionMassThreshold(1.8e-15);
	checkpoint->setAdvanceWholeCellClock(true);

	CellFateDecisionComponent* fate = plugins->newInstance<CellFateDecisionComponent>(model, "CellFate");
	fate->setWholeCellState(state);

	CellDivisionEvent* division = plugins->newInstance<CellDivisionEvent>(model, "Division");
	division->setWholeCellState(state);
	division->setDivisionMassThreshold(1.8e-15);
	division->setFtsZThreshold(0.0);
	division->setRandomSeed(330u);

	Dispose* steadySink = plugins->newInstance<Dispose>(model, "SteadySink");
	Dispose* divisionSink = plugins->newInstance<Dispose>(model, "DivisionSink");
	Dispose* starvedSink = plugins->newInstance<Dispose>(model, "StarvedSink");
	Dispose* deadSink = plugins->newInstance<Dispose>(model, "DeadSink");

	clock->connectTo(bioSimulate);
	bioSimulate->connectTo(projection);
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

	model->save("./models/WcmWholeCellMvp.gen");
	sim->start();

	std::cout << "whole_cell_mvp"
	          << " status=" << metabolism->getLastStatus()
	          << " nutrient=" << nutrient->getAmount()
	          << " energy=" << energy->getAmount()
		          << " ATP=" << state->getMetaboliteAmount("ATP")
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
