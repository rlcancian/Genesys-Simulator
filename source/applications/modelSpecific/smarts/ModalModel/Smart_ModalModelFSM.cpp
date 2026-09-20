#include "Smart_ModalModelFSM.h"

#include "kernel/simulator/Simulator.h"
#include "plugins/components/Logic/Create.h"
#include "plugins/components/Logic/Dispose.h"
#include "plugins/components/ModalModel/ModalModelDefault.h"
#include "plugins/components/ModalModel/FSMState.h"
#include "plugins/components/ModalModel/DefaultTransitionExtensions.h"
#include "plugins/data/ModalModel/EFSMNetwork.h"
#include "plugins/data/Logic/Variable.h"

Smart_ModalModelFSM::Smart_ModalModelFSM() {
}

int Smart_ModalModelFSM::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();

	Create* create = plugins->newInstance<Create>(model);
	create->setMaxCreations(1);
	ModalModelDefault* modal = new ModalModelDefault(model, "TrafficFSM");
	modal->setTimeDelayExpressionPerDispatch("1.0");
	Dispose* dispose = plugins->newInstance<Dispose>(model);

	EFSMNetwork* network = new EFSMNetwork(model, "TrafficNetwork");
	FSMState* red = plugins->newInstance<FSMState>(model, "Red");
	FSMState* green = plugins->newInstance<FSMState>(model, "Green");
	red->setInitialNode(true);
	red->setEntryActionExpression("signal=0");
	green->setEntryActionExpression("signal=1");
	network->addState(red);
	network->addState(green);
	network->setInitialState(red);

	EFSMTransition* r2g = new EFSMTransition(red, green, "RedToGreen");
	r2g->setGuardExpression("1");
	r2g->setOutputExpression("switches=switches+1");
	r2g->setPriority(0);
	network->addTransition(r2g);

	EFSMTransition* g2r = new EFSMTransition(green, red, "GreenToRed");
	g2r->setGuardExpression("1");
	g2r->setOutputExpression("switches=switches+1");
	g2r->setPriority(1);
	network->addTransition(g2r);

	modal->setNetwork(network);
	modal->setInputBinding(0, "1");
	modal->setOutputBinding(0, "switches");
	modal->addOutputExpressionReference(plugins->newInstance<Variable>(model, "switches"));

	create->getConnectionManager()->insert(modal);
	modal->getConnectionManager()->insert(dispose);

	model->getSimulation()->setReplicationLength(30, Util::TimeUnit::second);
	model->save("./models/Smart_ModalModelFSM.gen");
	model->getSimulation()->start();

	delete genesys;
	return 0;
}
