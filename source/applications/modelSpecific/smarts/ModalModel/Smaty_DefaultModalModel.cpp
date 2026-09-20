/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */


#include "Smart_DefaultModalModel.h"

// GEnSyS Simulator
#include "kernel/simulator/Simulator.h"

// Model Components
#include "plugins/components/Logic/Create.h"
#include "plugins/components/ModalModel/ModalModelDefault.h"
#include "plugins/components/Logic/Dispose.h"
#include "plugins/components/ModalModel/FSMState.h"
#include "plugins/components/ModalModel/DefaultTransitionExtensions.h"
#include "plugins/data/ModalModel/EFSMNetwork.h"

Smart_DefaultModalModel::Smart_DefaultModalModel() {
}

/**
 * Demonstrates ModalModelDefault attached to an EFSMNetwork.
 */
int Smart_DefaultModalModel::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();

	Create* create1 = plugins->newInstance<Create>(model);
	create1->setTimeBetweenCreationsExpression("1", Util::TimeUnit::microsecond);
	ModalModelDefault* dmm1 = plugins->newInstance<ModalModelDefault>(model);
	Dispose* dispose1 = plugins->newInstance<Dispose>(model);

	EFSMNetwork* network = new EFSMNetwork(model, "DefaultModalNetwork");
	FSMState* node1 = new FSMState(model, "N1");
	FSMState* node2 = new FSMState(model, "N2");
	node1->setInitialNode(true);
	network->addState(node1);
	network->addState(node2);
	network->setInitialState(node1);
	EFSMTransition* edge = new EFSMTransition(node1, node2, "N1_to_N2");
	edge->setGuardExpression("1");
	network->addTransition(edge);

	dmm1->setNetwork(network);
	dmm1->setInputBinding(0, "1");
	dmm1->setOutputBinding(0, "result");

	create1->getConnectionManager()->insert(dmm1);
	dmm1->getConnectionManager()->insert(dispose1);

	model->getSimulation()->setReplicationLength(100, Util::TimeUnit::microsecond);
	model->getSimulation()->setReplicationReportBaseTimeUnit(Util::TimeUnit::microsecond);
	model->save("./models/Smart_DefaultModalModel.gen");
	model->getSimulation()->start();
	delete genesys;
	return 0;
};
