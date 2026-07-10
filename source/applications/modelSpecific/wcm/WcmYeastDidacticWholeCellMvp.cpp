#include "WcmYeastDidacticWholeCellMvp.h"

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

WcmYeastDidacticWholeCellMvp::WcmYeastDidacticWholeCellMvp() {}

int WcmYeastDidacticWholeCellMvp::main(int argc, char** argv) {
	(void) argc;
	(void) argv;

	Simulator* genesys = new Simulator();
	this->setDefaultTraceHandlers(genesys->getTraceManager());
	genesys->getPluginManager()->autoInsertPlugins();

	Model* model = genesys->getModelManager()->newModel();
	PluginManager* plugins = genesys->getPluginManager();

	model->getInfos()->setName("Yeast-Oriented Didactic Whole-Cell MVP");
	model->getInfos()->setDescription(
		"Didactic Saccharomyces cerevisiae-inspired whole-cell MVP using a compact "
		"GEM-style FBA core, multi-rule compartment exchange, expression, growth, "
		"and lifecycle progression. This is not a canonical public yeast whole-cell model.");

	BioSpecies* glucoseCytosol = plugins->newInstance<BioSpecies>(model, "YeastGLC_c_i");
	glucoseCytosol->setInitialAmount(0.0);
	glucoseCytosol->setAmount(0.0);

	BioSpecies* oxygenMito = plugins->newInstance<BioSpecies>(model, "YeastO2_m_i");
	oxygenMito->setInitialAmount(0.0);
	oxygenMito->setAmount(0.0);

	BioSpecies* atpPool = plugins->newInstance<BioSpecies>(model, "YeastATP_i");
	atpPool->setInitialAmount(0.0);
	atpPool->setAmount(0.0);

	BioSpecies* ethanolCytosol = plugins->newInstance<BioSpecies>(model, "YeastEtOH_c_i");
	ethanolCytosol->setInitialAmount(0.0);
	ethanolCytosol->setAmount(0.0);

	MetabolicReaction* glucoseImport = plugins->newInstance<MetabolicReaction>(model, "YeastGlucoseTransportFlux");
	glucoseImport->addProduct("YeastGLC_c_i", 1.0);
	glucoseImport->setLowerBound(6.0);
	glucoseImport->setUpperBound(6.0);

	MetabolicReaction* oxygenImport = plugins->newInstance<MetabolicReaction>(model, "YeastOxygenTransportFlux");
	oxygenImport->addProduct("YeastO2_m_i", 1.0);
	oxygenImport->setLowerBound(0.0);
	oxygenImport->setUpperBound(2.0);

	MetabolicReaction* fermentation = plugins->newInstance<MetabolicReaction>(model, "YeastFermentationFlux");
	fermentation->addReactant("YeastGLC_c_i", 1.0);
	fermentation->addProduct("YeastATP_i", 1.0);
	fermentation->addProduct("YeastEtOH_c_i", 1.0);
	fermentation->setLowerBound(0.0);
	fermentation->setUpperBound(100.0);

	MetabolicReaction* respiration = plugins->newInstance<MetabolicReaction>(model, "YeastRespirationFlux");
	respiration->addReactant("YeastGLC_c_i", 1.0);
	respiration->addReactant("YeastO2_m_i", 1.0);
	respiration->addProduct("YeastATP_i", 3.0);
	respiration->setLowerBound(0.0);
	respiration->setUpperBound(100.0);

	MetabolicReaction* maintenance = plugins->newInstance<MetabolicReaction>(model, "YeastMaintenanceFlux");
	maintenance->addReactant("YeastATP_i", 1.0);
	maintenance->setLowerBound(1.0);
	maintenance->setUpperBound(1.0);

	MetabolicReaction* ethanolExport = plugins->newInstance<MetabolicReaction>(model, "YeastEthanolExportFlux");
	ethanolExport->addReactant("YeastEtOH_c_i", 1.0);
	ethanolExport->setLowerBound(0.0);
	ethanolExport->setUpperBound(100.0);

	MetabolicReaction* biomass = plugins->newInstance<MetabolicReaction>(model, "YeastBiomassFlux");
	biomass->addReactant("YeastATP_i", 1.0);
	biomass->setLowerBound(0.0);
	biomass->setUpperBound(100.0);

	MetabolicNetwork* network = plugins->newInstance<MetabolicNetwork>(model, "YeastDidacticNetwork");
	network->addReaction("YeastGlucoseTransportFlux");
	network->addReaction("YeastOxygenTransportFlux");
	network->addReaction("YeastFermentationFlux");
	network->addReaction("YeastRespirationFlux");
	network->addReaction("YeastMaintenanceFlux");
	network->addReaction("YeastEthanolExportFlux");
	network->addReaction("YeastBiomassFlux");
	network->setObjectiveReactionName("YeastBiomassFlux");
	network->setObjectiveSense("Maximize");
	network->setCompartment("cell");

	WholeCellState* state = plugins->newInstance<WholeCellState>(model, "YeastDidacticState");
	state->setCellMass(1.0e-15);
	state->setCellVolume(1.0e-3);
	state->setMoleculeCount("mRNA_HXT", 0);
	state->setMoleculeCount("mRNA_PFK", 0);
	state->setMoleculeCount("mRNA_ADH1", 0);
	state->setMoleculeCount("prot_HXT", 0);
	state->setMoleculeCount("prot_PFK", 0);
	state->setMoleculeCount("prot_ADH1", 0);
	state->setMoleculeCount("RNAP_free", 6);
	state->setMoleculeCount("ribosome_free", 8);
	state->setMetaboliteAmount("ATP", 0.0);
	state->setLifecyclePhase("newborn");

	Create* clock = plugins->newInstance<Create>(model, "YeastClock");
	clock->setTimeBetweenCreationsExpression("60", Util::TimeUnit::second);
	clock->setMaxCreations(8);

	MetabolicFluxBalance* fluxBalance = plugins->newInstance<MetabolicFluxBalance>(model, "YeastFluxBalance");
	fluxBalance->setMetabolicNetwork(network);

	MetabolicStateProjectionComponent* projection = plugins->newInstance<MetabolicStateProjectionComponent>(model, "YeastProjection");
	projection->setWholeCellState(state);
	projection->setFluxBalanceComponent(fluxBalance);
	projection->setObjectiveAsPathwayActivity("biomass_objective");
	projection->addCompartmentMetaboliteProjection("YeastGlucoseTransportFlux", "extracellular", "GLC_ext", 0.20, 0.0,
		MetabolicStateProjectionComponent::ProjectionUpdateMode::Accumulate);
	projection->addCompartmentMetaboliteProjection("YeastOxygenTransportFlux", "extracellular", "O2_ext", 0.25, 0.0,
		MetabolicStateProjectionComponent::ProjectionUpdateMode::Accumulate);
	projection->addCompartmentMetaboliteProjection("YeastFermentationFlux", "cytosol", "EtOH_c", 0.20, 0.0,
		MetabolicStateProjectionComponent::ProjectionUpdateMode::Accumulate);
	projection->addMetaboliteProjection("YeastFermentationFlux", "ATP", 0.15, 0.0,
		MetabolicStateProjectionComponent::ProjectionUpdateMode::Accumulate);
	projection->addCompartmentMetaboliteProjection("YeastFermentationFlux", "cytosol", "ATP_c", 0.15, 0.0,
		MetabolicStateProjectionComponent::ProjectionUpdateMode::Turnover, 0.20);
	projection->addMetaboliteProjection("YeastRespirationFlux", "ATP", 0.40, 0.0,
		MetabolicStateProjectionComponent::ProjectionUpdateMode::Accumulate);
	projection->addCompartmentMetaboliteProjection("YeastRespirationFlux", "mitochondria", "ATP_m", 0.50, 0.0,
		MetabolicStateProjectionComponent::ProjectionUpdateMode::Turnover, 0.20);
	projection->addPathwayProjection("YeastGlucoseTransportFlux", "glucose_transport_flux");
	projection->addPathwayProjection("YeastOxygenTransportFlux", "oxygen_transport_flux");
	projection->addPathwayProjection("YeastFermentationFlux", "fermentation_flux");
	projection->addPathwayProjection("YeastRespirationFlux", "respiration_flux");
	projection->addPathwayProjection("YeastEthanolExportFlux", "ethanol_export_flux");
	projection->addPathwayProjection("YeastBiomassFlux", "biomass_flux");

	CompartmentExchangeComponent* exchange = plugins->newInstance<CompartmentExchangeComponent>(model, "YeastExchange");
	exchange->setWholeCellState(state);
	exchange->addExchangeRule("glucose_import", "extracellular", "GLC_ext", "cytosol", "GLC_c", 1.0, 0.0,
		"glucose_transport_flux", 0.20, 2.0, true);
	exchange->addExchangeRule("oxygen_import", "extracellular", "O2_ext", "mitochondria", "O2_m", 1.0, 0.0,
		"oxygen_transport_flux", 0.50, 1.0, true);
	exchange->addExchangeRule("ethanol_export", "cytosol", "EtOH_c", "extracellular", "EtOH_ext", 1.0, 0.0,
		"ethanol_export_flux", 0.25, 1.5, true);

	ResourceAllocationComponent* allocation = plugins->newInstance<ResourceAllocationComponent>(model, "YeastAllocation");
	allocation->setWholeCellState(state);
	allocation->setRnapCountKey("RNAP_free");
	allocation->setRibosomeCountKey("ribosome_free");
	allocation->setMRNASpeciesPrefix("mRNA_");
	allocation->setProteinSpeciesPrefix("prot_");

	StochasticTranscription* transcription = plugins->newInstance<StochasticTranscription>(model, "YeastTranscription");
	transcription->setWholeCellState(state);
	transcription->setElongationRate(55.0);
	transcription->setMeanGeneLength(1200.0);
	transcription->setBindingProbability(0.35);
	transcription->setTimeWindow(60.0);
	transcription->setMRNASpeciesPrefix("mRNA_");
	transcription->setRnapCountKey("RNAP_free");
	transcription->setRandomSeed(601u);

	StochasticTranslation* translation = plugins->newInstance<StochasticTranslation>(model, "YeastTranslation");
	translation->setWholeCellState(state);
	translation->setElongationRate(18.0);
	translation->setMeanProteinLength(420.0);
	translation->setTimeWindow(60.0);
	translation->setMRNASpeciesPrefix("mRNA_");
	translation->setProteinSpeciesPrefix("prot_");
	translation->setRibosomeCountKey("ribosome_free");
	translation->setRandomSeed(602u);

	CellGrowthComponent* growth = plugins->newInstance<CellGrowthComponent>(model, "YeastGrowth");
	growth->setWholeCellState(state);
	growth->setGrowthRate(0.0027);
	growth->setDeltaT(60.0);
	growth->setDensity(1000.0);
	growth->setEnergyMetaboliteKey("ATP");
	growth->setEnergyHalfSaturation(1.5);

	CellCycleCheckpointComponent* checkpoint = plugins->newInstance<CellCycleCheckpointComponent>(model, "YeastCheckpoint");
	checkpoint->setWholeCellState(state);
	checkpoint->setDeltaT(60.0);
	checkpoint->setEnergyMetaboliteKey("ATP");
	checkpoint->setStarvationAtpThreshold(0.50);
	checkpoint->setCompartmentEnergyRegion("cytosol");
	checkpoint->setCompartmentEnergyMetaboliteKey("ATP_c");
	checkpoint->setCompartmentStarvationThreshold(0.40);
	checkpoint->setCriticalPathwayActivityKey("biomass_objective");
	checkpoint->setCriticalPathwayActivityThreshold(6.0);
	checkpoint->setDivisionMassThreshold(1.75e-15);
	checkpoint->setAdvanceWholeCellClock(true);

	CellFateDecisionComponent* fate = plugins->newInstance<CellFateDecisionComponent>(model, "YeastFate");
	fate->setWholeCellState(state);

	CellDivisionEvent* division = plugins->newInstance<CellDivisionEvent>(model, "YeastDivision");
	division->setWholeCellState(state);
	division->setDivisionMassThreshold(1.75e-15);
	division->setFtsZThreshold(0.0);
	division->setRandomSeed(603u);

	Dispose* steadySink = plugins->newInstance<Dispose>(model, "YeastSteadySink");
	Dispose* divisionSink = plugins->newInstance<Dispose>(model, "YeastDivisionSink");
	Dispose* starvedSink = plugins->newInstance<Dispose>(model, "YeastStarvedSink");
	Dispose* deadSink = plugins->newInstance<Dispose>(model, "YeastDeadSink");

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

	model->save("./models/WcmYeastDidacticWholeCellMvp.gen");
	sim->start();

	std::cout << "whole_cell_yeast_didactic_mvp"
	          << " scope=gem_inspired_not_canonical_wcm"
	          << " objective=" << fluxBalance->getLastObjectiveValue()
	          << " ATP=" << state->getMetaboliteAmount("ATP")
	          << " ATP_c=" << state->getCompartmentMetaboliteAmount("cytosol", "ATP_c")
	          << " ATP_m=" << state->getCompartmentMetaboliteAmount("mitochondria", "ATP_m")
	          << " GLC_c=" << state->getCompartmentMetaboliteAmount("cytosol", "GLC_c")
	          << " O2_m=" << state->getCompartmentMetaboliteAmount("mitochondria", "O2_m")
	          << " EtOH_ext=" << state->getCompartmentMetaboliteAmount("extracellular", "EtOH_ext")
	          << " fermentation_flux=" << state->getPathwayActivity("fermentation_flux")
	          << " respiration_flux=" << state->getPathwayActivity("respiration_flux")
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
