 /*
 * File:   Smart_BioNetworkAnalysis.cpp
 */

#include "Smart_BioNetworkAnalysis.h"

#include <iostream>
#include <iomanip>

#include "kernel/simulator/Simulator.h"
#include "plugins/data/BiochemicalSimulation/BioNetwork.h"
#include "plugins/data/BiochemicalSimulation/BioParameter.h"
#include "plugins/data/BiochemicalSimulation/BioReaction.h"
#include "plugins/data/BiochemicalSimulation/BioSpecies.h"
#include "tools/SimulationResultsDataset.h"
#include "../../../TraitsApp.h"

Smart_BioNetworkAnalysis::Smart_BioNetworkAnalysis() {
}

int Smart_BioNetworkAnalysis::main(int argc, char** argv) {
	(void) argc;
	(void) argv;

	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins();
	Model* model = genesys->getModelManager()->newModel();

	BioSpecies* substrate = plugins->newInstance<BioSpecies>(model, "Substrate");
	substrate->setInitialAmount(12.0);
	substrate->setAmount(12.0);
	BioSpecies* product = plugins->newInstance<BioSpecies>(model, "Product");
	product->setInitialAmount(0.0);
	product->setAmount(0.0);
	BioSpecies* enzyme = plugins->newInstance<BioSpecies>(model, "Enzyme");
	enzyme->setInitialAmount(1.5);
	enzyme->setAmount(1.5);
	enzyme->setConstant(true);

	BioParameter* kForward = plugins->newInstance<BioParameter>(model, "kForward");
	kForward->setValue(0.09);
	BioParameter* kReverse = plugins->newInstance<BioParameter>(model, "kReverse");
	kReverse->setValue(0.03);

	BioReaction* conversion = plugins->newInstance<BioReaction>(model, "Substrate_to_Product");
	conversion->addReactant("Substrate", 1.0);
	conversion->addProduct("Product", 1.0);
	conversion->addModifier("Enzyme");
	conversion->setKineticLawExpression("kForward * Enzyme * Substrate");
	conversion->setReverseKineticLawExpression("kReverse * Enzyme * Product");
	conversion->setReversible(true);

	BioNetwork* network = plugins->newInstance<BioNetwork>(model, "BioNetworkAnalysis");
	network->addSpecies("Substrate");
	network->addSpecies("Product");
	network->addSpecies("Enzyme");
	network->addReaction("Substrate_to_Product");

	std::string errorMessage;
	if (!network->simulate(0.0, 6.0, 0.02, errorMessage)) {
		std::cerr << "Smart_BioNetworkAnalysis failed to simulate: " << errorMessage << std::endl;
		delete genesys;
		return 1;
	}

	const BioSimulationResult& result = network->getLastSimulationResult();
	std::cout << "status=" << network->getLastStatus()
	          << " samples=" << result.sampleCount()
	          << " payload=" << network->getLastResponsePayload() << std::endl;

	SimulationResultsDataset substrateDataset;
	if (!network->getSpeciesTimeCourseDataset("Substrate", &substrateDataset, &errorMessage)) {
		std::cerr << "Species dataset failed: " << errorMessage << std::endl;
		delete genesys;
		return 1;
	}
	std::cout << "dataset(Substrate) observations=" << substrateDataset.observations.size();
	if (!substrateDataset.observations.empty()) {
		const auto& first = substrateDataset.observations.front();
		const auto& last = substrateDataset.observations.back();
		std::cout << " first=(" << first.time << "," << first.value << ")"
		          << " last=(" << last.time << "," << last.value << ")";
	}
	std::cout << std::endl;

	BioStoichiometryMatrix matrix;
	if (!network->getStoichiometryMatrix(&matrix, &errorMessage)) {
		std::cerr << "Stoichiometry matrix failed: " << errorMessage << std::endl;
		delete genesys;
		return 1;
	}
	std::cout << "stoichiometry species=" << matrix.speciesNames.size()
	          << " reactions=" << matrix.reactionNames.size() << std::endl;
	for (unsigned int i = 0; i < matrix.speciesNames.size(); ++i) {
		std::cout << "S[" << matrix.speciesNames[i] << "]=";
		for (unsigned int j = 0; j < matrix.reactionNames.size(); ++j) {
			std::cout << std::showpos << matrix.coefficient(i, j) << std::noshowpos;
			if (j + 1 < matrix.reactionNames.size()) {
				std::cout << ",";
			}
		}
		std::cout << std::endl;
	}

	BioReactionRateTimeCourse rates;
	if (!network->getReactionRateTimeCourse(&rates, &errorMessage)) {
		std::cerr << "Reaction rates failed: " << errorMessage << std::endl;
		delete genesys;
		return 1;
	}
	std::cout << "rates samples=" << rates.samples.size();
	if (!rates.samples.empty()) {
		const auto& first = rates.samples.front();
		const auto& last = rates.samples.back();
		std::cout << " firstNet=" << first.netRates.front()
		          << " lastNet=" << last.netRates.front();
	}
	std::cout << std::endl;

	BioSteadyStateCheck steady;
	if (!network->checkLastSampleSteadyState(0.05, &steady, &errorMessage)) {
		std::cerr << "Steady-state check failed: " << errorMessage << std::endl;
		delete genesys;
		return 1;
	}
	std::cout << "steady=" << (steady.steady ? "true" : "false")
	          << " maxAbsDerivative=" << steady.maxAbsoluteDerivative
	          << " tolerance=" << steady.tolerance << std::endl;

	BioSensitivityScan sensitivity;
	if (!network->scanLocalParameterSensitivity(0.01, 1.0e-6, &sensitivity, &errorMessage)) {
		std::cerr << "Sensitivity scan failed: " << errorMessage << std::endl;
		delete genesys;
		return 1;
	}
	std::cout << "sensitivity entries=" << sensitivity.entries.size() << std::endl;
	for (const BioParameterSensitivityEntry& entry : sensitivity.entries) {
		std::cout << "d(dX/dt)/d" << entry.parameterName
		          << " delta=" << entry.delta
		          << " maxAbs=" << entry.maxAbsoluteSensitivity << std::endl;
	}

	model->save("./models/Smart_BioNetworkAnalysis.gen");

	delete genesys;
	return 0;
}
