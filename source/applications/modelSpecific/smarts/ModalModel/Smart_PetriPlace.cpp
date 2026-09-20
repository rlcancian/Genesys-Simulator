#include "Smart_PetriPlace.h"

#include "kernel/simulator/Simulator.h"
#include "plugins/components/Logic/Create.h"
#include "plugins/components/Logic/Dispose.h"
#include "plugins/components/ModalModel/ModalModelDefault.h"
#include "plugins/components/ModalModel/PetriPlace.h"
#include "plugins/data/ModalModel/CPNArc.h"
#include "plugins/data/ModalModel/CPNTransition.h"
#include "plugins/data/ModalModel/ColoredPetriNetNetwork.h"

Smart_PetriPlace::Smart_PetriPlace() {
}

int Smart_PetriPlace::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();

	Create* create = plugins->newInstance<Create>(model);
	ModalModelDefault* modal = new ModalModelDefault(model, "SingleFlowPetri");
	Dispose* dispose = plugins->newInstance<Dispose>(model);

	ColoredPetriNetNetwork* network = new ColoredPetriNetNetwork(model, "SingleFlowNetwork");
	PetriPlace* source = new PetriPlace(model, "Source");
	PetriPlace* sink = new PetriPlace(model, "Sink");
	CPNTransition* moveBlue = new CPNTransition(model, "MoveBlue");
	source->setInitialNode(true);
	network->addPlace(source);
	network->addPlace(sink);
	network->addTransition(moveBlue);
	network->setInitialTokens(source, "blue", 3);

	CPNArc* inArc = new CPNArc(model, source, moveBlue, CPNArc::Direction::PlaceToTransition, "In");
	inArc->setInscription("blue", 1);
	CPNArc* outArc = new CPNArc(model, sink, moveBlue, CPNArc::Direction::TransitionToPlace, "Out");
	outArc->setInscription("blue", 1);
	network->addArc(inArc);
	network->addArc(outArc);

	modal->setNetwork(network);
	modal->setInputBinding(0, "1");
	modal->setOutputBinding(0, "fired");

	create->getConnectionManager()->insert(modal);
	modal->getConnectionManager()->insert(dispose);

	model->getSimulation()->setReplicationLength(10, Util::TimeUnit::second);
	model->save("./models/Smart_PetriPlace.gen");
	model->getSimulation()->start();

	delete genesys;
	return 0;
}
