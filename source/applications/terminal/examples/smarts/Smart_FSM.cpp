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
#include "../../../../plugins/data/Variable.h"
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
    // crete model
    Model *model = genesys->getModels()->newModel();
    PluginManager *plugins = genesys->getPlugins();
    Create *create1 = plugins->newInstance<Create>(model);

    FSM *fsm1 = plugins->newInstance<FSM>(model);

    Variable *var = plugins->newInstance<Variable>(model, "var");
    var->setInitialValue(0);

    // Creating states to represent when a variable in the model is even or odd
    FSMState *evenState = plugins->newInstance<FSMState>(model);
    // fsm1->states()->insert(evenState);
    FSMState *oddState = plugins->newInstance<FSMState>(model);
    // fsm1->states()->insert(oddState);

    // Creating transitions that check if the variable is even or odd
    // FSMTransition *evenToOdd = plugins->newInstance<FSMTransition>(model);
    // evenToOdd->setGuard("var % 2 == 1");
    // Add a function to be called whenever the transition is activated
    // Inside this function, we can do whatever we want, like changing variables or sending the entity to the next component
    // evenToOdd->onTransition(&Smart_FSM::onEvenToOdd);
    // fsm1->transitions()->insert(evenToOdd);
    // fsm1->insertTransition(evenToOdd, evenState, oddState);

    // This transition doesn't have a guard, so it's a default transition
    // It also doesn't have a side-effect (like setAttribute)
    // FSMTransition *evenToEven = plugins->newInstance<FSMTransition>(model);
    // fsm1->insertTransition(evenToEven, evenState, evenState);

    // FSMTransition *oddToEven = plugins->newInstance<FSMTransition>(model);
    // oddToEven->setGuard("var % 2 == 0");
    // oddToEven->onTransition(&Smart_FSM::onOddToEven);
    // fsm1->insertTransition(oddToEven, oddState, evenState);

    // FSMTransition *oddToOdd = plugins->newInstance<FSMTransition>(model);
    // fsm1->insertTransition(oddToOdd, oddState, oddState);

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
