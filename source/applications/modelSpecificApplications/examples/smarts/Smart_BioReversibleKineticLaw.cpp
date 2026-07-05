/*
 * File:   Smart_BioReversibleKineticLaw.cpp
 */

#include "Smart_BioReversibleKineticLaw.h"

#include <iostream>

#include "kernel/simulator/Simulator.h"
#include "plugins/data/BiochemicalSimulation/BioNetwork.h"
#include "plugins/data/BiochemicalSimulation/BioParameter.h"
#include "plugins/data/BiochemicalSimulation/BioReaction.h"
#include "plugins/data/BiochemicalSimulation/BioSpecies.h"
#include "../../../TraitsApp.h"

Smart_BioReversibleKineticLaw::Smart_BioReversibleKineticLaw() {
}

int Smart_BioReversibleKineticLaw::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins();
	Model* model = genesys->getModelManager()->newModel();

	BioSpecies* substrate = plugins->newInstance<BioSpecies>(model, "Substrate");
	substrate->setInitialAmount(10.0);
	substrate->setAmount(10.0);
	BioSpecies* product = plugins->newInstance<BioSpecies>(model, "Product");
	product->setInitialAmount(0.0);
	product->setAmount(0.0);
	BioSpecies* enzyme = plugins->newInstance<BioSpecies>(model, "Enzyme");
	enzyme->setInitialAmount(2.0);
	enzyme->setAmount(2.0);
	enzyme->setConstant(true);

	BioParameter* kForward = plugins->newInstance<BioParameter>(model, "kForward");
	kForward->setValue(0.08);
	BioParameter* kReverse = plugins->newInstance<BioParameter>(model, "kReverse");
	kReverse->setValue(0.02);

	BioReaction* reversibleEnzymaticStep = plugins->newInstance<BioReaction>(model, "Substrate_to_Product");
	reversibleEnzymaticStep->addReactant("Substrate", 1.0);
	reversibleEnzymaticStep->addProduct("Product", 1.0);
	reversibleEnzymaticStep->addModifier("Enzyme");
	reversibleEnzymaticStep->setKineticLawExpression("kForward * Enzyme * Substrate");
	reversibleEnzymaticStep->setReverseKineticLawExpression("kReverse * Enzyme * Product");
	reversibleEnzymaticStep->setReversible(true);

	BioNetwork* network = plugins->newInstance<BioNetwork>(model, "ReversibleKineticLawNetwork");
	network->addSpecies("Substrate");
	network->addSpecies("Product");
	network->addSpecies("Enzyme");
	network->addReaction("Substrate_to_Product");

	std::string errorMessage;
	if (!network->simulate(0.0, 8.0, 0.01, errorMessage)) {
		std::cerr << "Smart_BioReversibleKineticLaw failed: " << errorMessage << std::endl;
		delete genesys;
		return 1;
	}

	model->save("./models/Smart_BioReversibleKineticLaw.gen");
	std::cout << "Smart_BioReversibleKineticLaw status: " << network->getLastStatus() << std::endl;
	std::cout << "Smart_BioReversibleKineticLaw payload: " << network->getLastResponsePayload() << std::endl;
	std::cout << "Substrate=" << substrate->getAmount()
			<< " Product=" << product->getAmount()
			<< " Enzyme=" << enzyme->getAmount() << std::endl;

	delete genesys;
	return 0;
}
