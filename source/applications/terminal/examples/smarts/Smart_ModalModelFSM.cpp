#include "Smart_ModalModelFSM.h"

#include "kernel/simulator/Simulator.h"
#include "plugins/components/DiscreteProcessing/Create.h"
#include "plugins/components/DiscreteProcessing/Dispose.h"
#include "plugins/components/Network/ModalModelFSM.h"
#include "plugins/components/Network/FSMState.h"
#include "plugins/components/Network/DefaultTransitionExtensions.h"
#include "../../../TraitsApp.h"
#include "plugins/data/DiscreteProcessing/Variable.h"

Smart_ModalModelFSM::Smart_ModalModelFSM() {
}

int Smart_ModalModelFSM::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();

	Create* create = plugins->newInstance<Create>(model);
	create->setMaxCreations(1);
	ModalModelFSM* fsm = new ModalModelFSM(model, "TrafficFSM");
	fsm->setTimeDelayExpressionPerDispatch("1.0");
	Dispose* dispose = plugins->newInstance<Dispose>(model);

	FSMState* red = plugins->newInstance<FSMState>(model, "Red");
	FSMState* green = plugins->newInstance<FSMState>(model, "Green");
	red->setInitialNode(true);
	red->setEntryActionExpression("signal=0");
	green->setEntryActionExpression("signal=1");
	fsm->addNode(red);
	fsm->addNode(green);
	fsm->setEntryNode(red);
	//fsm->addActionExpressionReference(plugins->newInstance<Variable>(model, "signal"));


	EFSMTransition* r2g = new EFSMTransition(red, green, "RedToGreen");
	r2g->setGuardExpression("1");
	r2g->setOutputExpression("switches=switches+1");
	r2g->setPriority(0);
	fsm->addTransition(r2g);
	fsm->addOutputExpressionReference(plugins->newInstance<Variable>(model, "switches"));

	EFSMTransition* g2r = new EFSMTransition(green, red, "GreenToRed");
	g2r->setGuardExpression("1");
	g2r->setOutputExpression("switches=switches+1");
	g2r->setPriority(1);
	fsm->addTransition(g2r);

	create->getConnectionManager()->insert(fsm);
	fsm->getConnectionManager()->insert(fsm); // outputConnection[0] = normal output
	fsm->getConnectionManager()->insert(dispose); // outputConnection[1] = finishing output

	model->getSimulation()->setReplicationLength(30, Util::TimeUnit::second);
	model->save("./models/Smart_ModalModelFSM.gen");
	model->getSimulation()->start();

	delete genesys;
	return 0;
}
