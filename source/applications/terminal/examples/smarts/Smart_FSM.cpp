/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   Smart_FSM.cpp
 * Author: henrique.buss
 *
 * Created on 27 de Maio de 2022, 16:35
 */

#include "Smart_FSM.h"

// you have to included need libs

// GEnSyS Simulator
#include "../../../../kernel/simulator/Simulator.h"

// Model Components
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/FSM.h"
#include "../../../../plugins/data/FSMState.h"
#include "../../../../plugins/data/FSMTransition.h"
#include "../../../../plugins/components/Dispose.h"

Smart_FSM::Smart_FSM()
{
}

/**
 * This is the main function of the application.
 * It instanciates the simulator, builds a simulation model and then simulate that model.
 */
int Smart_FSM::main(int argc, char **argv)
{
    Simulator *genesys = new Simulator();
    this->setDefaultTraceHandlers(genesys->getTracer());
    this->insertFakePluginsByHand(genesys);
    // create model
    Model *model = genesys->getModels()->newModel();
    PluginManager *plugins = genesys->getPlugins();
    Create *create1 = plugins->newInstance<Create>(model);
    create1->setTimeBetweenCreationsExpression("1");
    create1->setMaxCreations(1);

    FSM *fsm1 = plugins->newInstance<FSM>(model, "FSM");

    FSMState *firstState = plugins->newInstance<FSMState>(model);
    fsm1->insertState(firstState);

    FSMState *secondState = plugins->newInstance<FSMState>(model);
    fsm1->insertState(secondState);

    FSMState *thirdState = plugins->newInstance<FSMState>(model);
    thirdState->setIsFinal(true);
    fsm1->insertState(thirdState);

    fsm1->setInitialState(firstState);

    FSMTransition *firstToSecond = plugins->newInstance<FSMTransition>(model);
    fsm1->insertTransition(firstToSecond, firstState, secondState);

    FSMTransition *secondToThird = plugins->newInstance<FSMTransition>(model);
    fsm1->insertTransition(secondToThird, secondState, thirdState);

    Dispose *dispose1 = plugins->newInstance<Dispose>(model);
    // connect model components to create a "workflow"
    create1->getConnections()->insert(fsm1);
    fsm1->getConnections()->insert(dispose1);
    // set options, save and simulate
    model->getSimulation()->setReplicationLength(60);
    model->save("./models/Smart_FSM.gen");
    model->getSimulation()->start();
    genesys->~Simulator();
    return 0;
};
