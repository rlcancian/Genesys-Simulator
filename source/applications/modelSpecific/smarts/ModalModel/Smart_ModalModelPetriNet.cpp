#include "Smart_ModalModelPetriNet.h"

#include "kernel/simulator/Simulator.h"
#include "plugins/components/Logic/Create.h"
#include "plugins/components/Logic/Dispose.h"
#include "plugins/components/ModalModel/ModalModelDefault.h"
#include "plugins/components/ModalModel/PetriPlace.h"
#include "plugins/data/ModalModel/CPNArc.h"
#include "plugins/data/ModalModel/CPNTransition.h"
#include "plugins/data/ModalModel/ColoredPetriNetNetwork.h"

Smart_ModalModelPetriNet::Smart_ModalModelPetriNet() {
}

int Smart_ModalModelPetriNet::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();

	Create* create = plugins->newInstance<Create>(model);
	ModalModelDefault* modal = new ModalModelDefault(model, "PetriFlow");
	Dispose* dispose = plugins->newInstance<Dispose>(model);

	ColoredPetriNetNetwork* network = new ColoredPetriNetNetwork(model, "PetriNetwork");
	PetriPlace* pIn = new PetriPlace(model, "P_In");
	PetriPlace* pOut = new PetriPlace(model, "P_Out");
	CPNTransition* moveRed = new CPNTransition(model, "MoveRedToken");
	pIn->setInitialNode(true);
	network->addPlace(pIn);
	network->addPlace(pOut);
	network->addTransition(moveRed);
	network->setInitialTokens(pIn, "red", 5);

	CPNArc* inArc = new CPNArc(model, pIn, moveRed, CPNArc::Direction::PlaceToTransition, "In");
	inArc->setInscription("red", 1);
	CPNArc* outArc = new CPNArc(model, pOut, moveRed, CPNArc::Direction::TransitionToPlace, "Out");
	outArc->setInscription("red", 1);
	network->addArc(inArc);
	network->addArc(outArc);

	modal->setNetwork(network);
	modal->setInputBinding(0, "1");
	modal->setOutputBinding(0, "fired");

	create->getConnectionManager()->insert(modal);
	modal->getConnectionManager()->insert(dispose);

	model->getSimulation()->setReplicationLength(20, Util::TimeUnit::second);
	model->save("./models/Smart_ModalModelPetriNet.gen");
	model->getSimulation()->start();

	delete genesys;
	return 0;
}
