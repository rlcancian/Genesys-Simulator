#include "WcmCompartmentTransportWholeCellMvp.h"

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
#include "plugins/components/WholeCellModeling/CompartmentExchangeComponent.h"
#include "plugins/components/WholeCellModeling/MetabolicStateProjectionComponent.h"
#include "plugins/components/WholeCellModeling/ResourceAllocationComponent.h"
#include "plugins/components/WholeCellModeling/StochasticTranscription.h"
#include "plugins/components/WholeCellModeling/StochasticTranslation.h"

#include "plugins/data/BiochemicalSimulation/BioSpecies.h"
#include "plugins/data/BiochemicalSimulation/MetabolicNetwork.h"
#include "plugins/data/BiochemicalSimulation/MetabolicReaction.h"
#include "plugins/data/WholeCellModeling/WholeCellState.h"

WcmCompartmentTransportWholeCellMvp::WcmCompartmentTransportWholeCellMvp() {}

int WcmCompartmentTransportWholeCellMvp::main(int argc, char** argv) {
	(void) argc;
	(void) argv;

	Simulator* genesys = new Simulator();
	this->setDefaultTraceHandlers(genesys->getTraceManager());
	genesys->getPluginManager()->autoInsertPlugins();

	Model* model = genesys->getModelManager()->newModel();
	PluginManager* plugins = genesys->getPluginManager();

	model->getInfos()->setName("Compartment-Transport Whole-Cell MVP");
	model->getInfos()->setDescription(
		"Whole-cell MVP using flux balance, extracellular-to-cytosol transport, "
		"compartment-aware metabolic projection, reusable exchange policies, and "
		"pathway-sensitive lifecycle checkpoints.");

	BioSpecies* atpExtracellular = plugins->newInstance<BioSpecies>(model, "FbaATP_ext_i");
	atpExtracellular->setInitialAmount(0.0);
	atpExtracellular->setAmount(0.0);

	BioSpecies* atpCytosol = plugins->newInstance<BioSpecies>(model, "FbaATP_c_i");
	atpCytosol->setInitialAmount(0.0);
	atpCytosol->setAmount(0.0);

	MetabolicReaction* uptake = plugins->newInstance<MetabolicReaction>(model, "FbaExternalUptake");
	uptake->addProduct("FbaATP_ext_i", 1.0);
	uptake->setLowerBound(0.0);
	uptake->setUpperBound(6.0);

	MetabolicReaction* transport = plugins->newInstance<MetabolicReaction>(model, "FbaTransportToCytosol");
	transport->addReactant("FbaATP_ext_i", 1.0);
	transport->addProduct("FbaATP_c_i", 1.0);
	transport->setLowerBound(0.0);
	transport->setUpperBound(6.0);

	MetabolicReaction* maintenance = plugins->newInstance<MetabolicReaction>(model, "FbaCytosolMaintenance");
	maintenance->addReactant("FbaATP_c_i", 1.0);
	maintenance->setLowerBound(1.0);
	maintenance->setUpperBound(1.0);

	MetabolicReaction* biomass = plugins->newInstance<MetabolicReaction>(model, "FbaCytosolBiomass");
	biomass->addReactant("FbaATP_c_i", 1.0);
	biomass->setLowerBound(0.0);
	biomass->setUpperBound(100.0);

	MetabolicNetwork* network = plugins->newInstance<MetabolicNetwork>(model, "FbaTransportWholeCellNetwork");
	network->addReaction("FbaExternalUptake");
	network->addReaction("FbaTransportToCytosol");
	network->addReaction("FbaCytosolMaintenance");
	network->addReaction("FbaCytosolBiomass");
	network->setObjectiveReactionName("FbaCytosolBiomass");
	network->setObjectiveSense("Maximize");
	network->setCompartment("cytosol");

	WholeCellState* state = plugins->newInstance<WholeCellState>(model, "TransportCellState");
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

	Create* clock = plugins->newInstance<Create>(model, "TransportClock");
	clock->setTimeBetweenCreationsExpression("60", Util::TimeUnit::second);
	clock->setMaxCreations(8);

	MetabolicFluxBalance* fluxBalance = plugins->newInstance<MetabolicFluxBalance>(model, "TransportFluxBalance");
	fluxBalance->setMetabolicNetwork(network);

	MetabolicStateProjectionComponent* projection = plugins->newInstance<MetabolicStateProjectionComponent>(model, "TransportFluxProjection");
	projection->setWholeCellState(state);
	projection->setFluxBalanceComponent(fluxBalance);
	projection->setObjectiveAsPathwayActivity("biomass_objective");
	projection->addCompartmentMetaboliteProjection("FbaExternalUptake", "extracellular", "ATP_ext", 0.25, 0.0,
		MetabolicStateProjectionComponent::ProjectionUpdateMode::Accumulate);
	projection->addMetaboliteProjection("FbaTransportToCytosol", "ATP", 0.20, 0.0,
		MetabolicStateProjectionComponent::ProjectionUpdateMode::Accumulate);
	projection->addPathwayProjection("FbaTransportToCytosol", "transport_flux");
	projection->addPathwayProjection("FbaCytosolBiomass", "biomass_flux");

	CompartmentExchangeComponent* exchange = plugins->newInstance<CompartmentExchangeComponent>(model, "TransportExchange");
	exchange->setWholeCellState(state);
	exchange->setSourceRegion("extracellular");
	exchange->setSourceMetaboliteKey("ATP_ext");
	exchange->setTargetRegion("cytosol");
	exchange->setTargetMetaboliteKey("ATP_c");
	exchange->setExchangeFraction(1.0);
	exchange->setDriverPathwayKey("transport_flux");
	exchange->setDriverScale(0.20);
	exchange->setMaxTransferAmount(2.0);
	exchange->setConserveMass(true);

	ResourceAllocationComponent* allocation = plugins->newInstance<ResourceAllocationComponent>(model, "TransportAllocation");
	allocation->setWholeCellState(state);
	allocation->setRnapCountKey("RNAP_free");
	allocation->setRibosomeCountKey("ribosome_free");
	allocation->setMRNASpeciesPrefix("mRNA_");
	allocation->setProteinSpeciesPrefix("prot_");

	StochasticTranscription* transcription = plugins->newInstance<StochasticTranscription>(model, "TransportTranscription");
	transcription->setWholeCellState(state);
	transcription->setElongationRate(50.0);
	transcription->setMeanGeneLength(900.0);
	transcription->setBindingProbability(0.30);
	transcription->setTimeWindow(60.0);
	transcription->setMRNASpeciesPrefix("mRNA_");
	transcription->setRnapCountKey("RNAP_free");
	transcription->setRandomSeed(501u);

	StochasticTranslation* translation = plugins->newInstance<StochasticTranslation>(model, "TransportTranslation");
	translation->setWholeCellState(state);
	translation->setElongationRate(16.0);
	translation->setMeanProteinLength(300.0);
	translation->setTimeWindow(60.0);
	translation->setMRNASpeciesPrefix("mRNA_");
	translation->setProteinSpeciesPrefix("prot_");
	translation->setRibosomeCountKey("ribosome_free");
	translation->setRandomSeed(502u);

	CellGrowthComponent* growth = plugins->newInstance<CellGrowthComponent>(model, "TransportGrowth");
	growth->setWholeCellState(state);
	growth->setGrowthRate(0.0025);
	growth->setDeltaT(60.0);
	growth->setDensity(1000.0);
	growth->setEnergyMetaboliteKey("ATP");
	growth->setEnergyHalfSaturation(1.0);

	CellCycleCheckpointComponent* checkpoint = plugins->newInstance<CellCycleCheckpointComponent>(model, "TransportCheckpoint");
	checkpoint->setWholeCellState(state);
	checkpoint->setDeltaT(60.0);
	checkpoint->setEnergyMetaboliteKey("ATP");
	checkpoint->setStarvationAtpThreshold(0.25);
	checkpoint->setCompartmentEnergyRegion("cytosol");
	checkpoint->setCompartmentEnergyMetaboliteKey("ATP_c");
	checkpoint->setCompartmentStarvationThreshold(0.75);
	checkpoint->setCriticalPathwayActivityKey("biomass_objective");
	checkpoint->setCriticalPathwayActivityThreshold(4.5);
	checkpoint->setDivisionMassThreshold(1.8e-15);
	checkpoint->setAdvanceWholeCellClock(true);

	CellFateDecisionComponent* fate = plugins->newInstance<CellFateDecisionComponent>(model, "TransportFate");
	fate->setWholeCellState(state);

	CellDivisionEvent* division = plugins->newInstance<CellDivisionEvent>(model, "TransportDivision");
	division->setWholeCellState(state);
	division->setDivisionMassThreshold(1.8e-15);
	division->setFtsZThreshold(0.0);
	division->setRandomSeed(503u);

	Dispose* steadySink = plugins->newInstance<Dispose>(model, "TransportSteadySink");
	Dispose* divisionSink = plugins->newInstance<Dispose>(model, "TransportDivisionSink");
	Dispose* starvedSink = plugins->newInstance<Dispose>(model, "TransportStarvedSink");
	Dispose* deadSink = plugins->newInstance<Dispose>(model, "TransportDeadSink");

	clock->connectTo(fluxBalance);
	fluxBalance->connectTo(projection);
	projection->connectTo(exchange);
	exchange->connectTo(allocation);
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

	model->save("./models/WcmCompartmentTransportWholeCellMvp.gen");
	sim->start();

	std::cout << "whole_cell_transport_mvp"
	          << " objective=" << fluxBalance->getLastObjectiveValue()
	          << " ATP=" << state->getMetaboliteAmount("ATP")
	          << " ATP_ext=" << state->getCompartmentMetaboliteAmount("extracellular", "ATP_ext")
	          << " ATP_c=" << state->getCompartmentMetaboliteAmount("cytosol", "ATP_c")
	          << " transport_flux=" << state->getPathwayActivity("transport_flux")
	          << " biomass_flux=" << state->getPathwayActivity("biomass_flux")
	          << " biomass_objective=" << state->getPathwayActivity("biomass_objective")
	          << " mass=" << state->getCellMass()
	          << " time=" << state->getCurrentTime()
	          << " phase=" << state->getLifecyclePhase()
	          << " generation=" << state->getGenerationCount()
	          << " viable=" << (state->isViable() ? "true" : "false")
	          << " divisions=" << division->getDivisionCount()
	          << std::endl;

	delete genesys;
	return 0;
}
