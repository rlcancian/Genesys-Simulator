/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/class.cc to edit this template
 */

/* 
 * File:   Smart_FinitStateMachine.cpp
 * Author: rlcancian
 * 
 * Created on 7 de agosto de 2022, 12:26
 */

#include "Smart_ExtendedFSM.h"


// you have to included need libs

// GEnSyS Simulator
#include "../../../../kernel/simulator/Simulator.h"

// Model Components
#include "../../../../plugins/components/Create.h"
//#include "../../../../plugins/components/ExtendedFinishStateMachine.h"
#include "../../../../plugins/components/OLD_FiniteStateMachine.h"
#include "../../../../plugins/components/Dispose.h"

Smart_ExtendedFSM::Smart_ExtendedFSM() {
}

/**
 * This is the main function of the application. 
 * It instanciates the simulator, builds a simulation model and then simulate that model.
 */
int Smart_ExtendedFSM::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	this->setDefaultTraceHandlers(genesys->getTracer());
	// crete model
	Model* model = genesys->getModels()->newModel();
	PluginManager* plugins = genesys->getPlugins();
	Create* create1 = plugins->newInstance<Create>(model);
    OLD_FiniteStateMachine* efsm1 = plugins->newInstance<OLD_FiniteStateMachine>(model);
	Dispose* dispose1 = plugins->newInstance<Dispose>(model);
	// connect model components to create a "workflow"
	create1->getConnections()->insert(efsm1);
	efsm1->getConnections()->insert(dispose1);
	// set options, save and simulate
	model->getSimulation()->setReplicationLength(60);
	model->save("./models/Smart_ExtendedFSM.gen");
	model->getSimulation()->start();
	delete genesys;
	return 0;
};

