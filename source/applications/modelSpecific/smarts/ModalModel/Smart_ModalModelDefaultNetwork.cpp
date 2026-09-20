#include "Smart_ModalModelDefaultNetwork.h"

#include "kernel/simulator/Simulator.h"
#include "plugins/components/Logic/Create.h"
#include "plugins/components/Logic/Dispose.h"
#include "plugins/components/ModalModel/ModalModelDefault.h"
#include "plugins/components/ModalModel/FSMState.h"
#include "plugins/components/ModalModel/DefaultTransitionExtensions.h"
#include "plugins/data/ModalModel/EFSMNetwork.h"

Smart_ModalModelDefaultNetwork::Smart_ModalModelDefaultNetwork() {
}

int Smart_ModalModelDefaultNetwork::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();

	Create* create = plugins->newInstance<Create>(model);
	ModalModelDefault* modal = new ModalModelDefault(model, "ModalDefault");
	Dispose* dispose = plugins->newInstance<Dispose>(model);

	EFSMNetwork* network = new EFSMNetwork(model, "DefaultBridgeNetwork");
	FSMState* n1 = new FSMState(model, "NodeA");
	FSMState* n2 = new FSMState(model, "NodeB");
	n1->setInitialNode(true);
	n2->setFinalNode(true);
	network->addState(n1);
	network->addState(n2);
	network->setInitialState(n1);

	EFSMTransition* t1 = new EFSMTransition(n1, n2, "A_to_B");
	t1->setGuardExpression("1");
	t1->setPriority(0);
	network->addTransition(t1);

	modal->setNetwork(network);
	modal->setInputBinding(0, "1");
	modal->setOutputBinding(0, "stepped");

	create->getConnectionManager()->insert(modal);
	modal->getConnectionManager()->insert(dispose);

	model->getSimulation()->setReplicationLength(20, Util::TimeUnit::second);
	model->save("./models/Smart_ModalModelDefaultNetwork.gen");
	model->getSimulation()->start();

	delete genesys;
	return 0;
}
