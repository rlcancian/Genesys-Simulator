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

#include "Smart_FSM_Transitions.h"

// you have to included need libs

// GEnSyS Simulator
#include "../../../../kernel/simulator/Simulator.h"

// Model Components
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/Assign.h"
#include "../../../../plugins/components/FSM.h"
#include "../../../../plugins/data/FSMState.h"
#include "../../../../plugins/data/FSMTransition.h"
#include "../../../../plugins/components/Dispose.h"

Smart_FSM_Transitions::Smart_FSM_Transitions()
{
}

void Smart_FSM_Transitions::onNewGuessTransition(Model *model, Entity *entity)
{
    model->parseExpression("guess = ROUND(min + (max - min) / 2)");
}

void Smart_FSM_Transitions::onSmallerTransition(Model *model, Entity *entity)
{
    model->parseExpression("min = ROUND(min + (max - min) / 2)");
}

void Smart_FSM_Transitions::onLargerTransition(Model *model, Entity *entity)
{
    model->parseExpression("max = ROUND(max - (max - min) / 2)");
}

/**
 * This is the main function of the application.
 * It instanciates the simulator, builds a simulation model and then simulate that model.
 */
int Smart_FSM_Transitions::main(int argc, char **argv)
{
    Simulator *genesys = new Simulator();
    this->setDefaultTraceHandlers(genesys->getTracer());
    this->insertFakePluginsByHand(genesys);
    // create model
    Model *model = genesys->getModels()->newModel();
    PluginManager *plugins = genesys->getPlugins();
    Create *create1 = plugins->newInstance<Create>(model);
    create1->setMaxCreations(1);

    Assign *assign1 = plugins->newInstance<Assign>(model);
    assign1->getAssignments()->insert(new Assignment(model, "min", "0", true));
    assign1->getAssignments()->insert(new Assignment(model, "max", "100", true));
    assign1->getAssignments()->insert(new Assignment(model, "guess", "50", true));
    assign1->getAssignments()->insert(new Assignment(model, "goal", "TRUNC(UNIF(0,100))", false));

    FSM *fsm1 = plugins->newInstance<FSM>(model, "FSM");

    FSMState *guessState = plugins->newInstance<FSMState>(model, "Guess");
    FSMState *smallerState = plugins->newInstance<FSMState>(model, "Smaller");
    FSMState *largerState = plugins->newInstance<FSMState>(model, "Larger");
    FSMState *equalState = plugins->newInstance<FSMState>(model, "Equal");

    equalState->setIsFinal(true);
    fsm1->setInitialState(guessState);

    fsm1->insertState(guessState);
    fsm1->insertState(smallerState);
    fsm1->insertState(largerState);
    fsm1->insertState(equalState);

    FSMTransition *newGuessTransition = plugins->newInstance<FSMTransition>(model, "Guess Transition");
    newGuessTransition->onTransition(Smart_FSM_Transitions::onNewGuessTransition);
    newGuessTransition->setDelayExpression("10");

    FSMTransition *smallerTransition = plugins->newInstance<FSMTransition>(model, "Smaller Transition");
    smallerTransition->setGuardExpression("guess < goal");
    smallerTransition->onTransition(Smart_FSM_Transitions::onSmallerTransition);

    FSMTransition *largerTransition = plugins->newInstance<FSMTransition>(model, "Larger Transition");
    largerTransition->setGuardExpression("guess > goal");
    largerTransition->onTransition(Smart_FSM_Transitions::onLargerTransition);

    FSMTransition *equalTransition = plugins->newInstance<FSMTransition>(model, "Equal Transition");

    fsm1->insertTransition(newGuessTransition, smallerState, guessState);
    fsm1->insertTransition(newGuessTransition, largerState, guessState);

    fsm1->insertTransition(smallerTransition, guessState, smallerState);
    fsm1->insertTransition(largerTransition, guessState, largerState);
    fsm1->insertTransition(equalTransition, guessState, equalState);

    Dispose *dispose1 = plugins->newInstance<Dispose>(model);
    // connect model components to create a "workflow"
    create1->getConnections()->insert(assign1);
    assign1->getConnections()->insert(fsm1);
    fsm1->getConnections()->insert(dispose1);
    // set options, save and simulate
    model->getSimulation()->setReplicationLength(60);
    model->save("./models/Smart_FSM_Transitions.gen");
    model->getSimulation()->start();
    genesys->~Simulator();
    return 0;
};
