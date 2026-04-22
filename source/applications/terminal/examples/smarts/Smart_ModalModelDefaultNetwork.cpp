#include "Smart_ModalModelDefaultNetwork.h"

#include "kernel/simulator/Simulator.h"
#include "plugins/components/DiscreteProcessing/Create.h"
#include "plugins/components/DiscreteProcessing/Dispose.h"
#include "plugins/components/ModalModel/ModalModelDefault.h"
#include "plugins/components/ModalModel/DefaultNode.h"
#include "../../../TraitsApp.h"

Smart_ModalModelDefaultNetwork::Smart_ModalModelDefaultNetwork() {
}

int Smart_ModalModelDefaultNetwork::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();

	Create* create = plugins->newInstance<Create>(model);
	ModalModelDefault* modal = new ModalModelDefault(model, "ModalDefault");
	Dispose* dispose = plugins->newInstance<Dispose>(model);

	DefaultNode* n1 = new DefaultNode(model, "NodeA");
	DefaultNode* n2 = new DefaultNode(model, "NodeB");
	n1->setInitialNode(true);
	n2->setFinalNode(true);
	modal->addNode(n1);
	modal->addNode(n2);
	modal->setEntryNode(n1);

	DefaultNodeTransition* t1 = new DefaultNodeTransition(n1, n2, "A_to_B");
	t1->setGuardExpression("1");
	t1->setPriority(0);
	modal->addTransition(t1);

	create->getConnectionManager()->insert(modal);
	modal->getConnectionManager()->insert(dispose);

	model->getSimulation()->setReplicationLength(20, Util::TimeUnit::second);
	model->save("./models/Smart_ModalModelDefaultNetwork.gen");
	model->getSimulation()->start();

	delete genesys;
	return 0;
}
