/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Smart_ProcessArena.cpp
 * Author: rlcancian
 * 
 * Created on 3 de Setembro de 2019, 18:34
 */

#include "Smart_ProcessArena.h"

// you have to included need libs

#include "../../../../kernel/simulator/EntityType.h"
#include "../../../../kernel/simulator/ModelSimulation.h"
#include "../../../../kernel/simulator/Simulator.h"

#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/Delay.h"
#include "../../../../plugins/components/Dispose.h"
#include "../../../TraitsApp.h"

Smart_ProcessArena::Smart_ProcessArena() {
}

/**
 * This is the main function of the application. 
 * It instanciates the simulator, builds a simulation model and then simulate that model.
 */
int Smart_ProcessArena::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();
	// create model

    EntityType* entityType = new EntityType(model, "Entity 1");
    Create* create = new Create(model);
    create->setDescription("Create 1");
    create->setEntityType(entityType);
    create->setTimeBetweenCreationsExpression("EXPO(1)");
    create->setTimeUnit(Util::TimeUnit::hour);

    Delay* delay = new Delay(model);
    delay->setDescription("Process 1");
    delay->setDelayExpression("tria(0.5, 1, 1.5)");
    delay->setDelayTimeUnit(Util::TimeUnit::hour);
    create->getConnections()->insert(delay);

    Dispose* dispose = new Dispose(model);
    dispose->setDescription("Dispose 1");
    delay->getConnections()->insert(dispose);

    ModelSimulation* simulation = model->getSimulation();
    simulation->setReplicationLength(10);
    simulation->setReplicationLengthTimeUnit(Util::TimeUnit::minute);
    simulation->setNumberOfReplications(300);
    simulation->setWarmUpPeriod(0.05);
    simulation->setWarmUpPeriodTimeUnit(Util::TimeUnit::minute);
    model->save("./models/Smart_ProcessArena.gen");
    model->getSimulation()->start();
    while (model->getSimulation()-> isPaused());
    
    delete genesys;
    return 0;
};
