#include "Smart_FSMState.h"

#include "kernel/simulator/Simulator.h"
#include "plugins/components/DiscreteProcessing/Create.h"
#include "plugins/components/DiscreteProcessing/Dispose.h"
#include "plugins/components/ModalModel/ModalModelFSM.h"
#include "plugins/components/ModalModel/FSMState.h"
#include "plugins/components/ModalModel/DefaultTransitionExtensions.h"
#include "../../../TraitsApp.h"

Smart_FSMState::Smart_FSMState() {
}

int Smart_FSMState::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();

	Create* create = plugins->newInstance<Create>(model);
	ModalModelFSM* modal = new ModalModelFSM(model, "SingleStateFSM");
	Dispose* dispose = plugins->newInstance<Dispose>(model);

	FSMState* s = new FSMState(model, "Idle");
	s->setInitialNode(true);
	s->setEntryActionExpression("enteredIdle=1");
	s->setExitActionExpression("leftIdle=1");
	modal->addNode(s);
	modal->setEntryNode(s);

	EFSMTransition* self = new EFSMTransition(s, s, "StayIdle");
	self->setGuardExpression("1");
	self->setOutputExpression("visits=visits+1");
	modal->addTransition(self);

	create->getConnectionManager()->insert(modal);
	modal->getConnectionManager()->insert(dispose);

	model->getSimulation()->setReplicationLength(10, Util::TimeUnit::second);
	model->save("./models/Smart_FSMState.gen");
	model->getSimulation()->start();

	delete genesys;
	return 0;
}
