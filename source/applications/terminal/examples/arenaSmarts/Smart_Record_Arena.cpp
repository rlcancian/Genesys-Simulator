/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Smart_Record_Arena.cpp
 * Author: rlcancian
 * 
 * Created on 3 de Setembro de 2019, 18:34
 */

#include "Smart_Record_Arena.h"

// you have to included need libs

#include "../../../../kernel/simulator/EntityType.h"
#include "../../../../kernel/simulator/ModelSimulation.h"
#include "../../../../kernel/simulator/Simulator.h"

#include "../../../../plugins/components/Assign.h"
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/Delay.h"
#include "../../../../plugins/components/Dispose.h"
#include "../../../../plugins/components/Record.h"
#include "../../../TraitsApp.h"

Smart_Record_Arena::Smart_Record_Arena() {
}

/**
 * This is the main function of the application. 
 * It instanciates the simulator, builds a simulation model and then simulate that model.
 */
int Smart_Record_Arena::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();
	// create model

    EntityType* entityType = new EntityType(model, "Person");
    Create* create = new Create(model);
    create->setDescription("Enter Store");
    create->setEntityType(entityType);
    create->setTimeBetweenCreationsExpression("EXPO(5)");
    create->setTimeUnit(Util::TimeUnit::minute);

    Assign* assign = new Assign(model);
    assign->setDescription("Mark Entry Time");
    Assignment* assignment = new Assignment("timeIn", "tnow");
    assign->getAssignments()->insert(assignment);
    new Attribute(model, "timeIn");
    create->getConnectionManager()->insert(assign);

    Delay* delay = new Delay(model);
    delay->setDescription("Browse");
    delay->setDelayExpression("tria(3, 7, 11)");
    delay->setDelayTimeUnit(Util::TimeUnit::minute);
    assign->getConnectionManager()->insert(delay);

    Record* record = new Record(model);
    record->setDescription("Time in Store");
    record->setExpression("timeIn");
    record->setExpressionName("Time in Store");
    delay->getConnectionManager()->insert(record);

    Dispose* dispose = new Dispose(model);
    dispose->setDescription("Leave Store");
    record->getConnectionManager()->insert(dispose);

    ModelSimulation* simulation = model->getSimulation();
    simulation->setReplicationLength(10);
    simulation->setReplicationLengthTimeUnit(Util::TimeUnit::minute);
    simulation->setNumberOfReplications(300);
    simulation->setWarmUpPeriod(0.05);
    simulation->setWarmUpPeriodTimeUnit(Util::TimeUnit::minute);
	model->save("./models/Smart_Record_Arena.gen");
    model->getSimulation()->start();
    while (model->getSimulation()-> isPaused());

    delete genesys;
    return 0;
};
