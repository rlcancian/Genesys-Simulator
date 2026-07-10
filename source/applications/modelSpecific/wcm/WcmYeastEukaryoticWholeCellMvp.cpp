#include "WcmYeastEukaryoticWholeCellMvp.h"

#include <iostream>

#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/model/Model.h"

#include "plugins/components/BiochemicalSimulation/MetabolicFluxBalance.h"
#include "plugins/components/Logic/Create.h"
#include "plugins/components/Logic/Dispose.h"
#include "plugins/components/WholeCellModeling/CellDivisionEvent.h"
#include "plugins/components/WholeCellModeling/CellFateDecisionComponent.h"
#include "plugins/components/WholeCellModeling/CellGrowthComponent.h"
#include "plugins/components/WholeCellModeling/CompartmentExchangeComponent.h"
#include "plugins/components/WholeCellModeling/EukaryoticCellCycleComponent.h"
#include "plugins/components/WholeCellModeling/MetabolicStateProjectionComponent.h"
#include "plugins/components/WholeCellModeling/ResourceAllocationComponent.h"
#include "plugins/components/WholeCellModeling/StochasticTranscription.h"
#include "plugins/components/WholeCellModeling/StochasticTranslation.h"

#include "plugins/data/BiochemicalSimulation/BioSpecies.h"
#include "plugins/data/BiochemicalSimulation/MetabolicNetwork.h"
#include "plugins/data/BiochemicalSimulation/MetabolicReaction.h"
#include "plugins/data/WholeCellModeling/BioCompartment.h"
#include "plugins/data/WholeCellModeling/WholeCellState.h"

WcmYeastEukaryoticWholeCellMvp::WcmYeastEukaryoticWholeCellMvp() {}

int WcmYeastEukaryoticWholeCellMvp::main(int argc, char** argv) {
	(void) argc;
	(void) argv;

	Simulator* genesys = new Simulator();
	this->setDefaultTraceHandlers(genesys->getTraceManager());
	genesys->getPluginManager()->autoInsertPlugins();

	Model* model = genesys->getModelManager()->newModel();
	PluginManager* plugins = genesys->getPluginManager();

	model->getInfos()->setName("Yeast Eukaryotic Whole-Cell MVP");
	model->getInfos()->setDescription(
		"Didactic Saccharomyces cerevisiae-inspired eukaryotic whole-cell MVP with "
		"explicit compartments, FBA-driven metabolism, expression, growth, "
		"and simplified G1/S/G2/M progression. This is not a canonical public yeast WCM.");

	BioCompartment* extracellular = plugins->newInstance<BioCompartment>(model, "extracellular");
	extracellular->setCompartmentType("extracellular");
	extracellular->setMembraneBounded(false);
	extracellular->setVolumeFraction(1.0);
	extracellular->setRole("environment");

	BioCompartment* cytosol = plugins->newInstance<BioCompartment>(model, "cytosol");
	cytosol->setCompartmentType("cytosol");
	cytosol->setMembraneBounded(false);
	cytosol->setVolumeFraction(0.70);
	cytosol->setRole("central_metabolism");

	BioCompartment* nucleus = plugins->newInstance<BioCompartment>(model, "nucleus");
	nucleus->setCompartmentType("nucleus");
	nucleus->setParentCompartmentName("cytosol");
	nucleus->setMembraneBounded(true);
	nucleus->setVolumeFraction(0.08);
	nucleus->setRole("replication_and_transcription");

	BioCompartment* mitochondria = plugins->newInstance<BioCompartment>(model, "mitochondria");
	mitochondria->setCompartmentType("mitochondrion");
	mitochondria->setParentCompartmentName("cytosol");
	mitochondria->setMembraneBounded(true);
	mitochondria->setVolumeFraction(0.12);
	mitochondria->setCopyNumber(4u);
	mitochondria->setRole("respiration");

	BioCompartment* bud = plugins->newInstance<BioCompartment>(model, "bud");
	bud->setCompartmentType("bud");
	bud->setParentCompartmentName("cytosol");
	bud->setMembraneBounded(true);
	bud->setVolumeFraction(0.15);
	bud->setRole("daughter_compartment");

	BioSpecies* glucoseCytosol = plugins->newInstance<BioSpecies>(model, "YeastEukGLC_c_i");
	glucoseCytosol->setInitialAmount(0.0);
	glucoseCytosol->setAmount(0.0);

	BioSpecies* oxygenMito = plugins->newInstance<BioSpecies>(model, "YeastEukO2_m_i");
	oxygenMito->setInitialAmount(0.0);
	oxygenMito->setAmount(0.0);

	BioSpecies* atpPool = plugins->newInstance<BioSpecies>(model, "YeastEukATP_i");
	atpPool->setInitialAmount(0.0);
	atpPool->setAmount(0.0);

	BioSpecies* ethanolCytosol = plugins->newInstance<BioSpecies>(model, "YeastEukEtOH_c_i");
	ethanolCytosol->setInitialAmount(0.0);
	ethanolCytosol->setAmount(0.0);

	MetabolicReaction* glucoseImport = plugins->newInstance<MetabolicReaction>(model, "YeastEukGlucoseTransportFlux");
	glucoseImport->addProduct("YeastEukGLC_c_i", 1.0);
	glucoseImport->setLowerBound(8.0);
	glucoseImport->setUpperBound(8.0);

	MetabolicReaction* oxygenImport = plugins->newInstance<MetabolicReaction>(model, "YeastEukOxygenTransportFlux");
	oxygenImport->addProduct("YeastEukO2_m_i", 1.0);
	oxygenImport->setLowerBound(0.0);
	oxygenImport->setUpperBound(3.0);

	MetabolicReaction* fermentation = plugins->newInstance<MetabolicReaction>(model, "YeastEukFermentationFlux");
	fermentation->addReactant("YeastEukGLC_c_i", 1.0);
	fermentation->addProduct("YeastEukATP_i", 1.0);
	fermentation->addProduct("YeastEukEtOH_c_i", 1.0);
	fermentation->setLowerBound(0.0);
	fermentation->setUpperBound(100.0);

	MetabolicReaction* respiration = plugins->newInstance<MetabolicReaction>(model, "YeastEukRespirationFlux");
	respiration->addReactant("YeastEukGLC_c_i", 1.0);
	respiration->addReactant("YeastEukO2_m_i", 1.0);
	respiration->addProduct("YeastEukATP_i", 4.0);
	respiration->setLowerBound(0.0);
	respiration->setUpperBound(100.0);

	MetabolicReaction* maintenance = plugins->newInstance<MetabolicReaction>(model, "YeastEukMaintenanceFlux");
	maintenance->addReactant("YeastEukATP_i", 1.0);
	maintenance->setLowerBound(1.0);
	maintenance->setUpperBound(1.0);

	MetabolicReaction* ethanolExport = plugins->newInstance<MetabolicReaction>(model, "YeastEukEthanolExportFlux");
	ethanolExport->addReactant("YeastEukEtOH_c_i", 1.0);
	ethanolExport->setLowerBound(0.0);
	ethanolExport->setUpperBound(100.0);

	MetabolicReaction* biomass = plugins->newInstance<MetabolicReaction>(model, "YeastEukBiomassFlux");
	biomass->addReactant("YeastEukATP_i", 1.0);
	biomass->setLowerBound(0.0);
	biomass->setUpperBound(100.0);

	MetabolicNetwork* network = plugins->newInstance<MetabolicNetwork>(model, "YeastEukaryoticDidacticNetwork");
	network->addReaction("YeastEukGlucoseTransportFlux");
	network->addReaction("YeastEukOxygenTransportFlux");
	network->addReaction("YeastEukFermentationFlux");
	network->addReaction("YeastEukRespirationFlux");
	network->addReaction("YeastEukMaintenanceFlux");
	network->addReaction("YeastEukEthanolExportFlux");
	network->addReaction("YeastEukBiomassFlux");
	network->setObjectiveReactionName("YeastEukBiomassFlux");
	network->setObjectiveSense("Maximize");
	network->setCompartment("cell");

	WholeCellState* state = plugins->newInstance<WholeCellState>(model, "YeastEukaryoticState");
	state->setCellMass(1.05e-15);
	state->setCellVolume(1.1e-3);
	state->setMoleculeCount("mRNA_HXT", 0);
	state->setMoleculeCount("mRNA_CDC28", 0);
	state->setMoleculeCount("mRNA_CLN3", 0);
	state->setMoleculeCount("prot_HXT", 0);
	state->setMoleculeCount("prot_CDC28", 0);
	state->setMoleculeCount("prot_CLN3", 0);
	state->setMoleculeCount("RNAP_free", 7);
	state->setMoleculeCount("ribosome_free", 10);
	state->setMetaboliteAmount("ATP", 0.0);
	state->setLifecyclePhase("newborn");

	Create* clock = plugins->newInstance<Create>(model, "YeastEukClock");
	clock->setTimeBetweenCreationsExpression("60", Util::TimeUnit::second);
	clock->setMaxCreations(10);

	MetabolicFluxBalance* fluxBalance = plugins->newInstance<MetabolicFluxBalance>(model, "YeastEukFluxBalance");
	fluxBalance->setMetabolicNetwork(network);

	MetabolicStateProjectionComponent* projection = plugins->newInstance<MetabolicStateProjectionComponent>(model, "YeastEukProjection");
	projection->setWholeCellState(state);
	projection->setFluxBalanceComponent(fluxBalance);
	projection->setObjectiveAsPathwayActivity("biomass_objective");
	projection->addMetaboliteProjection("YeastEukFermentationFlux", "ATP", 0.12, 0.0,
		MetabolicStateProjectionComponent::ProjectionUpdateMode::Accumulate);
	projection->addMetaboliteProjection("YeastEukRespirationFlux", "ATP", 0.40, 0.0,
		MetabolicStateProjectionComponent::ProjectionUpdateMode::Accumulate);
	projection->addCompartmentMetaboliteProjection("YeastEukRespirationFlux", "mitochondria", "ATP_m", 0.60, 0.0,
		MetabolicStateProjectionComponent::ProjectionUpdateMode::Turnover, 0.20);
	projection->addCompartmentMetaboliteProjection("YeastEukFermentationFlux", "cytosol", "ATP_c", 0.20, 0.0,
		MetabolicStateProjectionComponent::ProjectionUpdateMode::Turnover, 0.15);
	projection->addCompartmentMetaboliteProjection("YeastEukFermentationFlux", "cytosol", "EtOH_c", 0.20, 0.0,
		MetabolicStateProjectionComponent::ProjectionUpdateMode::Accumulate);
	projection->addPathwayProjection("YeastEukGlucoseTransportFlux", "glucose_transport_flux");
	projection->addPathwayProjection("YeastEukOxygenTransportFlux", "oxygen_transport_flux");
	projection->addPathwayProjection("YeastEukFermentationFlux", "fermentation_flux");
	projection->addPathwayProjection("YeastEukRespirationFlux", "respiration_flux");
	projection->addPathwayProjection("YeastEukEthanolExportFlux", "ethanol_export_flux");
	projection->addPathwayProjection("YeastEukBiomassFlux", "biomass_flux");

	CompartmentExchangeComponent* exchange = plugins->newInstance<CompartmentExchangeComponent>(model, "YeastEukExchange");
	exchange->setWholeCellState(state);
	exchange->addExchangeRule("glucose_import", "extracellular", "GLC_ext", "cytosol", "GLC_c", 1.0, 0.0,
		"glucose_transport_flux", 0.20, 2.5, true);
	exchange->addExchangeRule("oxygen_import", "extracellular", "O2_ext", "mitochondria", "O2_m", 1.0, 0.0,
		"oxygen_transport_flux", 0.50, 1.5, true);
	exchange->addExchangeRule("mito_to_cyt_atp", "mitochondria", "ATP_m", "cytosol", "ATP_c", 0.30, 0.10,
		"respiration_flux", 0.15, 1.0, true);
	exchange->addExchangeRule("cyt_to_bud_atp", "cytosol", "ATP_c", "bud", "ATP_bud", 0.20, 0.05,
		"biomass_flux", 0.10, 0.8, true);
	exchange->addExchangeRule("ethanol_export", "cytosol", "EtOH_c", "extracellular", "EtOH_ext", 1.0, 0.0,
		"ethanol_export_flux", 0.20, 1.5, true);

	ResourceAllocationComponent* allocation = plugins->newInstance<ResourceAllocationComponent>(model, "YeastEukAllocation");
	allocation->setWholeCellState(state);
	allocation->setRnapCountKey("RNAP_free");
	allocation->setRibosomeCountKey("ribosome_free");
	allocation->setMRNASpeciesPrefix("mRNA_");
	allocation->setProteinSpeciesPrefix("prot_");

	StochasticTranscription* transcription = plugins->newInstance<StochasticTranscription>(model, "YeastEukTranscription");
	transcription->setWholeCellState(state);
	transcription->setElongationRate(60.0);
	transcription->setMeanGeneLength(1400.0);
	transcription->setBindingProbability(0.30);
	transcription->setTimeWindow(60.0);
	transcription->setMRNASpeciesPrefix("mRNA_");
	transcription->setRnapCountKey("RNAP_free");
	transcription->setRandomSeed(611u);

	StochasticTranslation* translation = plugins->newInstance<StochasticTranslation>(model, "YeastEukTranslation");
	translation->setWholeCellState(state);
	translation->setElongationRate(20.0);
	translation->setMeanProteinLength(500.0);
	translation->setTimeWindow(60.0);
	translation->setMRNASpeciesPrefix("mRNA_");
	translation->setProteinSpeciesPrefix("prot_");
	translation->setRibosomeCountKey("ribosome_free");
	translation->setRandomSeed(612u);

	CellGrowthComponent* growth = plugins->newInstance<CellGrowthComponent>(model, "YeastEukGrowth");
	growth->setWholeCellState(state);
	growth->setGrowthRate(0.0030);
	growth->setDeltaT(60.0);
	growth->setDensity(1000.0);
	growth->setEnergyMetaboliteKey("ATP");
	growth->setEnergyHalfSaturation(1.5);

	EukaryoticCellCycleComponent* cycle = plugins->newInstance<EukaryoticCellCycleComponent>(model, "YeastEukCycle");
	cycle->setWholeCellState(state);
	cycle->setDeltaT(60.0);
	cycle->setAdvanceWholeCellClock(true);
	cycle->setGlobalAtpThreshold(0.8);
	cycle->setCytosolicAtpThreshold(0.6);
	cycle->setMitochondrialAtpThreshold(0.5);
	cycle->setBudAtpThreshold(0.3);
	cycle->setGrowthFluxThreshold(6.0);
	cycle->setRespirationFluxThreshold(2.0);
	cycle->setBudProgressRate(0.55);
	cycle->setDnaReplicationRate(0.48);
	cycle->setSpindleAssemblyRate(0.45);
	cycle->setMitoticExitRate(0.45);

	CellFateDecisionComponent* fate = plugins->newInstance<CellFateDecisionComponent>(model, "YeastEukFate");
	fate->setWholeCellState(state);

	CellDivisionEvent* division = plugins->newInstance<CellDivisionEvent>(model, "YeastEukDivision");
	division->setWholeCellState(state);
	division->setDivisionMassThreshold(1.85e-15);
	division->setFtsZThreshold(0.0);
	division->setRandomSeed(613u);

	Dispose* steadySink = plugins->newInstance<Dispose>(model, "YeastEukSteadySink");
	Dispose* divisionSink = plugins->newInstance<Dispose>(model, "YeastEukDivisionSink");
	Dispose* arrestedSink = plugins->newInstance<Dispose>(model, "YeastEukArrestedSink");
	Dispose* deadSink = plugins->newInstance<Dispose>(model, "YeastEukDeadSink");

	clock->connectTo(fluxBalance);
	fluxBalance->connectTo(projection);
	projection->connectTo(exchange);
	exchange->connectTo(allocation);
	allocation->connectTo(transcription);
	transcription->connectTo(translation);
	translation->connectTo(growth);
	growth->connectTo(cycle);
	cycle->connectTo(fate);
	fate->connectTo(steadySink);
	fate->connectTo(division);
	fate->connectTo(arrestedSink);
	fate->connectTo(deadSink);
	division->connectTo(steadySink);
	division->connectTo(divisionSink);

	ModelSimulation* sim = model->getSimulation();
	sim->setNumberOfReplications(1);
	sim->setReplicationLength(600.0, Util::TimeUnit::second);
	sim->setReplicationReportBaseTimeUnit(Util::TimeUnit::second);
	model->getTracer()->setTraceLevel(TraceManager::Level::L2_results);

	model->save("./models/WcmYeastEukaryoticWholeCellMvp.gen");
	sim->start();

	std::cout << "whole_cell_yeast_eukaryotic_mvp"
	          << " scope=didactic_eukaryotic_not_canonical_wcm"
	          << " objective=" << fluxBalance->getLastObjectiveValue()
	          << " ATP=" << state->getMetaboliteAmount("ATP")
	          << " ATP_c=" << state->getCompartmentMetaboliteAmount("cytosol", "ATP_c")
	          << " ATP_m=" << state->getCompartmentMetaboliteAmount("mitochondria", "ATP_m")
	          << " ATP_bud=" << state->getCompartmentMetaboliteAmount("bud", "ATP_bud")
	          << " biomass_flux=" << state->getPathwayActivity("biomass_flux")
	          << " respiration_flux=" << state->getPathwayActivity("respiration_flux")
	          << " bud_progress=" << state->getPathwayActivity("bud_growth_progress")
	          << " dna_progress=" << state->getPathwayActivity("dna_replication_progress")
	          << " spindle_progress=" << state->getPathwayActivity("spindle_assembly_progress")
	          << " mitotic_exit_progress=" << state->getPathwayActivity("mitotic_exit_progress")
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
