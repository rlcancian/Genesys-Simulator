/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */


#include "Smart_DefaultModalModel.h"

// you have to included need libs

// GEnSyS Simulator
#include "../../../../kernel/simulator/Simulator.h"

// Model Components
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/DefaultModalModel.h"
#include "../../../../plugins/components/Dispose.h"
#include "../../../TraitsApp.h"

Smart_DefaultModalModel::Smart_DefaultModalModel() {
}

/**
 * This is the main function of the application.
 * It instanciates the simulator, builds a simulation model and then simulate that model.
 */
int Smart_DefaultModalModel::main(int argc, char** argv) {
    Simulator* genesys = new Simulator();
    genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
    setDefaultTraceHandlers(genesys->getTraceManager());
    PluginManager* plugins = genesys->getPluginManager();
    plugins->autoInsertPlugins("autoloadplugins.txt");
    Model* model = genesys->getModelManager()->newModel();
    // create model
    Create* create1 = plugins->newInstance<Create>(model);
    create1->setTimeBetweenCreationsExpression("1", Util::TimeUnit::microsecond);
    DefaultModalModel* dmm1 = plugins->newInstance<DefaultModalModel>(model);
    DefaultNode* node1 = plugins->newInstance<DefaultNode>(model);
    DefaultNode* node2 = plugins->newInstance<DefaultNode>(model);
    DefaultNode* node3 = plugins->newInstance<DefaultNode>(model);
    DefaultNode* node4 = plugins->newInstance<DefaultNode>(model);
    dmm1->addNode(node1);
    dmm1->addNode(node2);
    dmm1->addNode(node3);
    dmm1->addNode(node4);
    dmm1->addTransition(new DefaultNodeTransition(node1, node1));
    dmm1->addTransition(new DefaultNodeTransition(node1, node2));
    dmm1->addTransition(new DefaultNodeTransition(node1, node3));
    dmm1->addTransition(new DefaultNodeTransition(node2, node4));
    dmm1->addTransition(new DefaultNodeTransition(node3, node4));
    dmm1->addTransition(new DefaultNodeTransition(node4, node4));
    dmm1->addTransition(new DefaultNodeTransition(node4, node1));
    Dispose* dispose1 = plugins->newInstance<Dispose>(model);
    // connect model components to create a "workflow"
    create1->getConnectionManager()->insert(dmm1);
    dmm1->getConnectionManager()->insert(dispose1);
    // set options, save and simulate
    model->getSimulation()->setReplicationLength(100, Util::TimeUnit::microsecond);
    model->getSimulation()->setReplicationReportBaseTimeUnit(Util::TimeUnit::microsecond);
    model->save("./models/Smart_DefaultModalModel.gen");
    model->getSimulation()->start();
    delete genesys;
    return 0;
};

