#include "Smart_PetriPlace.h"

#include "../../../../kernel/simulator/Simulator.h"
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/Dispose.h"
#include "../../../../plugins/components/ModalModelPetriNet.h"
#include "../../../../plugins/components/network/PetriPlace.h"
#include "../../../../plugins/components/network/DefaultTransitionExtensions.h"
#include "../../../TraitsApp.h"

Smart_PetriPlace::Smart_PetriPlace() {
}

int Smart_PetriPlace::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();

	Create* create = plugins->newInstance<Create>(model);
	ModalModelPetriNet* modal = new ModalModelPetriNet(model, "SingleFlowPetri");
	Dispose* dispose = plugins->newInstance<Dispose>(model);

	PetriPlace* source = new PetriPlace(model, "Source");
	PetriPlace* sink = new PetriPlace(model, "Sink");
	source->setInitialNode(true);
	source->addTokens(3, "blue");
	modal->addNode(source);
	modal->addNode(sink);
	modal->setEntryNode(source);

	PetriTransition* moveBlue = new PetriTransition(source, sink, "MoveBlue");
	moveBlue->setInputArcWeight("blue", 1);
	moveBlue->setOutputArcWeight("blue", 1);
	modal->addTransition(moveBlue);

	create->getConnectionManager()->insert(modal);
	modal->getConnectionManager()->insert(dispose);

	model->getSimulation()->setReplicationLength(10, Util::TimeUnit::second);
	model->save("./models/Smart_PetriPlace.gen");
	model->getSimulation()->start();

	delete genesys;
	return 0;
}
