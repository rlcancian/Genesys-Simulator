#include "WcmBioNetworkBridge.h"

#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/model/Model.h"

#include "plugins/components/BiochemicalSimulation/BioSimulate.h"
#include "plugins/components/Logic/Create.h"
#include "plugins/components/Logic/Dispose.h"
#include "plugins/components/WholeCellModeling/BioStateProjectionComponent.h"
#include "plugins/components/WholeCellModeling/StochasticReactionComponent.h"
#include "plugins/components/WholeCellModeling/StochasticTranscription.h"
#include "plugins/components/WholeCellModeling/StochasticTranslation.h"

#include "plugins/data/BiochemicalSimulation/BioNetwork.h"
#include "plugins/data/BiochemicalSimulation/BioParameter.h"
#include "plugins/data/BiochemicalSimulation/BioReaction.h"
#include "plugins/data/BiochemicalSimulation/BioSpecies.h"
#include "plugins/data/WholeCellModeling/StochasticReactionRule.h"
#include "plugins/data/WholeCellModeling/WholeCellState.h"

WcmBioNetworkBridge::WcmBioNetworkBridge() {}

int WcmBioNetworkBridge::main(int argc, char** argv) {
	(void)argc;
	(void)argv;

	Simulator* genesys = new Simulator();
	this->setDefaultTraceHandlers(genesys->getTraceManager());
	genesys->getPluginManager()->autoInsertPlugins();

	Model* model = genesys->getModelManager()->newModel();
	PluginManager* plugins = genesys->getPluginManager();

	model->getInfos()->setName("WCM BioNetwork Bridge");
	model->getInfos()->setDescription(
		"Didactic bridge from deterministic BioNetwork metabolism to stochastic "
		"whole-cell gene-expression components. Unitless toy parameters.");

	// Deterministic biochemical layer: nutrient is converted into a generic energy pool.
	BioSpecies* nutrient = plugins->newInstance<BioSpecies>(model, "Nutrient");
	nutrient->setInitialAmount(120.0);
	nutrient->setAmount(120.0);

	BioSpecies* energy = plugins->newInstance<BioSpecies>(model, "EnergyPool");
	energy->setInitialAmount(0.0);
	energy->setAmount(0.0);

	BioParameter* uptakeRate = plugins->newInstance<BioParameter>(model, "kUptake");
	uptakeRate->setValue(0.004);

	BioReaction* nutrientUptake = plugins->newInstance<BioReaction>(model, "Nutrient_to_Energy");
	nutrientUptake->addReactant("Nutrient", 1.0);
	nutrientUptake->addProduct("EnergyPool", 1.0);
	nutrientUptake->setKineticLawExpression("kUptake * Nutrient");

	BioNetwork* metabolism = plugins->newInstance<BioNetwork>(model, "DeterministicMetabolism");
	metabolism->addSpecies("Nutrient");
	metabolism->addSpecies("EnergyPool");
	metabolism->addReaction("Nutrient_to_Energy");

	BioSimulate* simulateMetabolism = plugins->newInstance<BioSimulate>(model, "DeterministicMetabolismStep");
	simulateMetabolism->setBioNetwork(metabolism);
	simulateMetabolism->setUseNetworkTimeWindow(true);

	// Stochastic whole-cell layer consumes the projected BioNetwork output at runtime.
	WholeCellState* state = plugins->newInstance<WholeCellState>(model, "CellState");
	state->setMoleculeCount("EnergyPool", 0);
	state->setMoleculeCount("RNAP_free", 1);
	state->setMoleculeCount("ribosome_free", 1);
	state->setMetaboliteAmount("ATP", 0.0);

	BioStateProjectionComponent* projection = plugins->newInstance<BioStateProjectionComponent>(model, "EnergyProjection");
	projection->setWholeCellState(state);
	projection->addMetaboliteProjection("EnergyPool", "ATP", 1.0);
	projection->addMoleculeProjection("EnergyPool", "RNAP_free", 0.25);
	projection->addMoleculeProjection("EnergyPool", "ribosome_free", 0.5);

	const std::vector<std::string> genes = {"geneA", "geneB"};
	for (const std::string& gene : genes) {
		state->setMoleculeCount("mRNA_" + gene, 0);
		state->setMoleculeCount("prot_" + gene, 0);

		StochasticReactionRule* degradeMRNA = plugins->newInstance<StochasticReactionRule>(
			model, "R_deg_mRNA_" + gene);
		degradeMRNA->addReactant("mRNA_" + gene, 1);
		degradeMRNA->setRateConstant(0.01);

		StochasticReactionRule* degradeProtein = plugins->newInstance<StochasticReactionRule>(
			model, "R_deg_prot_" + gene);
		degradeProtein->addReactant("prot_" + gene, 1);
		degradeProtein->setRateConstant(0.001);
	}

	Create* clock = plugins->newInstance<Create>(model, "SimClock");
	clock->setTimeBetweenCreationsExpression("60", Util::TimeUnit::second);
	clock->setMaxCreations(1);

	StochasticTranscription* transcription = plugins->newInstance<StochasticTranscription>(model, "Transcription");
	transcription->setWholeCellState(state);
	transcription->setElongationRate(50.0);
	transcription->setMeanGeneLength(900.0);
	transcription->setBindingProbability(0.20);
	transcription->setTimeWindow(60.0);
	transcription->setMRNASpeciesPrefix("mRNA_");
	transcription->setRnapCountKey("RNAP_free");
	transcription->setRandomSeed(1101u);

	StochasticTranslation* translation = plugins->newInstance<StochasticTranslation>(model, "Translation");
	translation->setWholeCellState(state);
	translation->setElongationRate(16.0);
	translation->setMeanProteinLength(300.0);
	translation->setTimeWindow(60.0);
	translation->setMRNASpeciesPrefix("mRNA_");
	translation->setProteinSpeciesPrefix("prot_");
	translation->setRibosomeCountKey("ribosome_free");
	translation->setRandomSeed(1202u);

	StochasticReactionComponent* degradation = plugins->newInstance<StochasticReactionComponent>(
		model, "Degradation_SSA");
	degradation->setWholeCellState(state);
	degradation->setTimeWindow(60.0);
	degradation->setRandomSeed(1303u);

	Dispose* sink = plugins->newInstance<Dispose>(model, "Done");

	clock->connectTo(simulateMetabolism);
	simulateMetabolism->connectTo(projection);
	projection->connectTo(transcription);
	transcription->connectTo(translation);
	translation->connectTo(degradation);
	degradation->connectTo(sink);

	ModelSimulation* sim = model->getSimulation();
	sim->setNumberOfReplications(1);
	sim->setReplicationLength(600.0, Util::TimeUnit::second);
	sim->setReplicationReportBaseTimeUnit(Util::TimeUnit::second);
	model->getTracer()->setTraceLevel(TraceManager::Level::L2_results);

	model->save("./models/WcmBioNetworkBridge.gen");
	sim->start();

	std::cout << "final_metabolism status=" << metabolism->getLastStatus()
	          << " nutrient=" << nutrient->getAmount()
	          << " energy=" << energy->getAmount()
	          << " ATP=" << state->getMetaboliteAmount("ATP")
	          << " RNAP_free=" << state->getMoleculeCount("RNAP_free")
	          << " ribosome_free=" << state->getMoleculeCount("ribosome_free")
	          << std::endl;

	delete genesys;
	return 0;
}
