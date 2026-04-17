#include "Smart_Modalmodel.h"

/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   Smart_ModelModel.cpp
 * Author: rlcancian
 *
 * Created on 30 de Setembro de 2025, 17:46
 */


// you have to included need libs

// GEnSyS Simulator
#include "kernel/simulator/Simulator.h"

// Model Components
#include "plugins/components/DiscreteProcessing/Create.h"
#include "plugins/components/Network/ModalModelDefault.h"
#include "plugins/components/DiscreteProcessing/Dispose.h"
#include "../../../TraitsApp.h"

Smart_ModalModel::Smart_ModalModel() {

}

/**
 * This is the main function of the application.
 * It instanciates the simulator, builds a simulation model and then simulate that model.
 */
int Smart_ModalModel::main(int argc, char** argv) {
    Simulator* genesys = new Simulator();
    genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
    setDefaultTraceHandlers(genesys->getTraceManager());
    PluginManager* plugins = genesys->getPluginManager();
    plugins->autoInsertPlugins("autoloadplugins.txt");
    Model* model = genesys->getModelManager()->newModel();
    // create model
    Create* create1 = plugins->newInstance<Create>(model);
    ModalModelDefault* modalmodel1 = plugins->newInstance<ModalModelDefault>(model);
    Dispose* dispose1 = plugins->newInstance<Dispose>(model);
    // connect model components to create a "workflow"
    create1->getConnectionManager()->insert(modalmodel1);
    modalmodel1->getConnectionManager()->insert(dispose1);
    // set options, save and simulate
    model->getSimulation()->setReplicationLength(60, Util::TimeUnit::second);
    model->getSimulation()->setTerminatingCondition("count(Dispose_1.CountNumberIn)>30");
    model->save("./models/Smart_Dummy.gen");
    model->getSimulation()->start();
    delete genesys;
    return 0;
};
