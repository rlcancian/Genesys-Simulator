#include "Smart_FSMState.h"

#include "kernel/simulator/Simulator.h"
#include "plugins/components/Logic/Create.h"
#include "plugins/components/Logic/Dispose.h"
#include "plugins/components/ModalModel/ModalModelDefault.h"
#include "plugins/components/ModalModel/FSMState.h"
#include "plugins/components/ModalModel/DefaultTransitionExtensions.h"
#include "plugins/data/ModalModel/EFSMNetwork.h"

Smart_FSMState::Smart_FSMState() {
}

int Smart_FSMState::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();

	Create* create = plugins->newInstance<Create>(model);
	ModalModelDefault* modal = new ModalModelDefault(model, "SingleStateFSM");
	Dispose* dispose = plugins->newInstance<Dispose>(model);

	EFSMNetwork* network = new EFSMNetwork(model, "IdleNetwork");
	FSMState* s = new FSMState(model, "Idle");
	s->setInitialNode(true);
	s->setEntryActionExpression("enteredIdle=1");
	s->setExitActionExpression("leftIdle=1");
	network->addState(s);
	network->setInitialState(s);

	EFSMTransition* self = new EFSMTransition(s, s, "StayIdle");
	self->setGuardExpression("1");
	self->setOutputExpression("visits=visits+1");
	network->addTransition(self);

	modal->setNetwork(network);
	modal->setInputBinding(0, "1");
	modal->setOutputBinding(0, "visits");

	create->getConnectionManager()->insert(modal);
	modal->getConnectionManager()->insert(dispose);

	model->getSimulation()->setReplicationLength(10, Util::TimeUnit::second);
	model->save("./models/Smart_FSMState.gen");
	model->getSimulation()->start();

	delete genesys;
	return 0;
}
