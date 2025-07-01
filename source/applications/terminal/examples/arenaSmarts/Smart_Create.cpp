/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Smart_Create.cpp
 * Author: rlcancian
 * 
 * Created on 3 de Setembro de 2019, 18:34
 */

#include "Smart_Create.h"

#include "../../../../kernel/simulator/EntityType.h"
#include "../../../../kernel/simulator/ModelSimulation.h"
#include "../../../../kernel/simulator/Simulator.h"

// Model Components
#include "../../../../plugins/components/Assign.h"
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/Delay.h"
#include "../../../../plugins/components/Dispose.h"
#include "../../../TraitsApp.h"

Smart_Create::Smart_Create() {
}

/** 
 * This is the main function of the application. 
 * It instanciates the simulator, builds a simulation model and then simulate that model.
 */
int Smart_Create::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();
	// create model
	EntityType* entityType = new EntityType(model, "Customers");
    Create* create = new Create(model);
    create->setDescription("Create Module");
    create->setEntityType(entityType);
    create->setTimeBetweenCreationsExpression("EXPO(1)");
    create->setTimeUnit(Util::TimeUnit::minute);
    
    Assign* assign = new Assign(model);
    assign->setDescription("Assign");
    Assignment* assignment = new Assignment("processTime", "NORM(10, 2)");
    assign->getAssignments()->insert(assignment);
    new Attribute(model, "processTime");
    create->getConnections()->insert(assign);
    
    Delay* delay = new Delay(model);
    delay->setDescription("Process");
    delay->setDelayExpression("processTime");
    delay->setDelayTimeUnit(Util::TimeUnit::minute);
    assign->getConnections()->insert(delay);
    
    Dispose* dispose = new Dispose(model);
    dispose->setDescription("Dispose");
    delay->getConnections()->insert(dispose);
    
    ModelSimulation* simulation = model->getSimulation();
    simulation->setReplicationLength(10);
    simulation->setReplicationLengthTimeUnit(Util::TimeUnit::minute);
    simulation->setNumberOfReplications(3);
    simulation->setWarmUpPeriod(0.05);
    simulation->setWarmUpPeriodTimeUnit(Util::TimeUnit::minute);
    model->save("./models/Smart_Create.gen");
    model->getSimulation()->start();
    while (model->getSimulation()-> isPaused());
    
    delete genesys;
    return 0;
};
