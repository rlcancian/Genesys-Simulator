/*
 * File:   Smart_BioReversibleMassAction.cpp
 */

#include "Smart_BioReversibleMassAction.h"

#include <iostream>

#include "kernel/simulator/Simulator.h"
#include "plugins/data/BiochemicalSimulation/BioNetwork.h"
#include "plugins/data/BiochemicalSimulation/BioParameter.h"
#include "plugins/data/BiochemicalSimulation/BioReaction.h"
#include "plugins/data/BiochemicalSimulation/BioSpecies.h"
#include "../../../TraitsApp.h"

Smart_BioReversibleMassAction::Smart_BioReversibleMassAction() {
}

int Smart_BioReversibleMassAction::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();

	BioSpecies* a = plugins->newInstance<BioSpecies>(model, "A");
	a->setInitialAmount(10.0);
	a->setAmount(10.0);
	BioSpecies* b = plugins->newInstance<BioSpecies>(model, "B");
	b->setInitialAmount(0.0);
	b->setAmount(0.0);

	BioParameter* kForward = plugins->newInstance<BioParameter>(model, "kForward");
	kForward->setValue(0.1);
	BioParameter* kReverse = plugins->newInstance<BioParameter>(model, "kReverse");
	kReverse->setValue(0.05);

	BioReaction* reversibleConversion = plugins->newInstance<BioReaction>(model, "A_to_B");
	reversibleConversion->addReactant("A", 1.0);
	reversibleConversion->addProduct("B", 1.0);
	reversibleConversion->setRateConstantParameterName("kForward");
	reversibleConversion->setReverseRateConstantParameterName("kReverse");
	reversibleConversion->setReversible(true);

	BioNetwork* network = plugins->newInstance<BioNetwork>(model, "ReversibleMassActionNetwork");
	network->addSpecies("A");
	network->addSpecies("B");
	network->addReaction("A_to_B");

	std::string errorMessage;
	if (!network->simulate(0.0, 10.0, 0.01, errorMessage)) {
		std::cerr << "Smart_BioReversibleMassAction failed: " << errorMessage << std::endl;
		delete genesys;
		return 1;
	}

	model->save("./models/Smart_BioReversibleMassAction.gen");
	std::cout << "Smart_BioReversibleMassAction status: " << network->getLastStatus() << std::endl;
	std::cout << "Smart_BioReversibleMassAction payload: " << network->getLastResponsePayload() << std::endl;
	std::cout << "A=" << a->getAmount() << " B=" << b->getAmount() << std::endl;

	delete genesys;
	return 0;
}
