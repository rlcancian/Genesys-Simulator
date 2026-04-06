#include "Smart_ModalModelPetriNet.h"

#include "../../../../kernel/simulator/Simulator.h"
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/Dispose.h"
#include "../../../../plugins/components/ModalModelPetriNet.h"
#include "../../../../plugins/components/network/PetriPlace.h"
#include "../../../../plugins/components/network/DefaultTransitionExtensions.h"
#include "../../../TraitsApp.h"

Smart_ModalModelPetriNet::Smart_ModalModelPetriNet() {
}

int Smart_ModalModelPetriNet::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();

	Create* create = plugins->newInstance<Create>(model);
	ModalModelPetriNet* petri = new ModalModelPetriNet(model, "PetriFlow");
	Dispose* dispose = plugins->newInstance<Dispose>(model);

	PetriPlace* pIn = new PetriPlace(model, "P_In");
	PetriPlace* pOut = new PetriPlace(model, "P_Out");
	pIn->setInitialNode(true);
	pIn->addTokens(5, "red");
	petri->addNode(pIn);
	petri->addNode(pOut);
	petri->setEntryNode(pIn);

	PetriTransition* t = new PetriTransition(pIn, pOut, "MoveRedToken");
	t->setInputArcWeight("red", 1);
	t->setOutputArcWeight("red", 1);
	t->setPriority(0);
	petri->addTransition(t);

	create->getConnectionManager()->insert(petri);
	petri->getConnectionManager()->insert(dispose);

	model->getSimulation()->setReplicationLength(20, Util::TimeUnit::second);
	model->save("./models/Smart_ModalModelPetriNet.gen");
	model->getSimulation()->start();

	delete genesys;
	return 0;
}
