#include "Smart_Modalmodel.h"

/*
 * File:   Smart_ModelModel.cpp
 * Author: rlcancian
 *
 * Created on 30 de Setembro de 2025, 17:46
 */

#include "kernel/simulator/Simulator.h"
#include "plugins/components/Logic/Create.h"
#include "plugins/components/ModalModel/ModalModelDefault.h"
#include "plugins/components/Logic/Dispose.h"
#include "plugins/components/ModalModel/FSMState.h"
#include "plugins/components/ModalModel/DefaultTransitionExtensions.h"
#include "plugins/data/ModalModel/EFSMNetwork.h"

Smart_ModalModel::Smart_ModalModel() {
}

int Smart_ModalModel::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();

	Create* create1 = plugins->newInstance<Create>(model);
	ModalModelDefault* modalmodel1 = plugins->newInstance<ModalModelDefault>(model);
	Dispose* dispose1 = plugins->newInstance<Dispose>(model);

	EFSMNetwork* network = new EFSMNetwork(model, "DemoNetwork");
	FSMState* idle = new FSMState(model, "Idle");
	idle->setInitialNode(true);
	network->addState(idle);
	network->setInitialState(idle);
	EFSMTransition* stay = new EFSMTransition(idle, idle, "Stay");
	stay->setGuardExpression("1");
	network->addTransition(stay);

	modalmodel1->setNetwork(network);
	modalmodel1->setInputBinding(0, "1");
	modalmodel1->setOutputBinding(0, "tick");

	create1->getConnectionManager()->insert(modalmodel1);
	modalmodel1->getConnectionManager()->insert(dispose1);

	model->getSimulation()->setReplicationLength(60, Util::TimeUnit::second);
	model->getSimulation()->setTerminatingCondition("count(Dispose_1.CountNumberIn)>30");
	model->save("./models/Smart_ModalModel.gen");
	model->getSimulation()->start();
	delete genesys;
	return 0;
};
