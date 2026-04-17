/*
 * File:   Smart_BioKineticLawRegulation.cpp
 */

#include "Smart_BioKineticLawRegulation.h"

#include <iostream>

#include "kernel/simulator/Simulator.h"
#include "plugins/data/BiochemicalSimulation/BioNetwork.h"
#include "plugins/data/BiochemicalSimulation/BioParameter.h"
#include "plugins/data/BiochemicalSimulation/BioReaction.h"
#include "plugins/data/BiochemicalSimulation/BioSpecies.h"
#include "../../../TraitsApp.h"

Smart_BioKineticLawRegulation::Smart_BioKineticLawRegulation() {
}

int Smart_BioKineticLawRegulation::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();

	BioSpecies* protein = plugins->newInstance<BioSpecies>(model, "Protein");
	protein->setInitialAmount(0.0);
	protein->setAmount(0.0);
	BioSpecies* activator = plugins->newInstance<BioSpecies>(model, "Activator");
	activator->setInitialAmount(3.0);
	activator->setAmount(3.0);
	activator->setConstant(true);

	BioParameter* synthesisRate = plugins->newInstance<BioParameter>(model, "kSynth");
	synthesisRate->setValue(2.0);
	BioParameter* degradationRate = plugins->newInstance<BioParameter>(model, "kDeg");
	degradationRate->setValue(0.4);

	BioReaction* synthesis = plugins->newInstance<BioReaction>(model, "RegulatedProteinSynthesis");
	synthesis->addProduct("Protein", 1.0);
	synthesis->addModifier("Activator");
	synthesis->setKineticLawExpression("kSynth * Activator / (1 + Activator)");

	BioReaction* degradation = plugins->newInstance<BioReaction>(model, "ProteinDegradation");
	degradation->addReactant("Protein", 1.0);
	degradation->setKineticLawExpression("kDeg * Protein");

	BioNetwork* network = plugins->newInstance<BioNetwork>(model, "KineticLawRegulationNetwork");
	network->addSpecies("Protein");
	network->addSpecies("Activator");
	network->addReaction("RegulatedProteinSynthesis");
	network->addReaction("ProteinDegradation");

	std::string errorMessage;
	if (!network->simulate(0.0, 5.0, 0.01, errorMessage)) {
		std::cerr << "Smart_BioKineticLawRegulation failed: " << errorMessage << std::endl;
		delete genesys;
		return 1;
	}

	model->save("./models/Smart_BioKineticLawRegulation.gen");
	std::cout << "Smart_BioKineticLawRegulation status: " << network->getLastStatus() << std::endl;
	std::cout << "Smart_BioKineticLawRegulation payload: " << network->getLastResponsePayload() << std::endl;
	std::cout << "Protein=" << protein->getAmount() << " Activator=" << activator->getAmount() << std::endl;

	delete genesys;
	return 0;
}
