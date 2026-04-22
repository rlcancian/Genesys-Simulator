
/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   Smart_Expression.cpp
 * Author: rlcancian
 *
 * Created on 3 de Setembro de 2019, 18:34
 */

#include "Smart_Expression.h"

#include "kernel/simulator/EntityType.h"
#include "kernel/simulator/ModelSimulation.h"
#include "kernel/simulator/Simulator.h"

#include "plugins/components/DiscreteProcessing/Assign.h"
#include "plugins/components/DiscreteProcessing/Create.h"
#include "plugins/components/Decisions/Decide.h"
#include "plugins/components/DiscreteProcessing/Delay.h"
#include "plugins/components/DiscreteProcessing/Dispose.h"
#include "../../../TraitsApp.h"

Smart_Expression::Smart_Expression() {
}

/**
 * This is the main function of the application.
 * It instanciates the simulator, builds a simulation model and then simulate that model.
 */
int Smart_Expression::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();
	// create model

    EntityType* entityType = plugins->newInstance<EntityType>(model, "Package");
    Create* create = plugins->newInstance<Create>(model);
    create->setDescription("Packages Arrive");
    create->setEntityType(entityType);
    create->setTimeBetweenCreationsExpression("EXPO(1)");
    create->setTimeUnit(Util::TimeUnit::minute);

    Assign* assign = plugins->newInstance<Assign>(model);
    assign->setDescription("The packages are weighted");
    Assignment* assignment = new Assignment("productWeight", "NORM(100, 5)");
    assign->getAssignments()->insert(assignment);
    plugins->newInstance<Attribute>(model, "productWeight");
    create->connectTo(assign);

    Delay* delay = plugins->newInstance<Delay>(model);
    delay->setDescription("The packages are processed");
    delay->setDelayExpression("productWeight * 0.33 + 5");
    delay->setDelayTimeUnit(Util::TimeUnit::minute);
    assign->connectTo(delay);

    Decide* decide = plugins->newInstance<Decide>(model);
    decide->setDescription("Send Package to correct department");
    decide->getConditions()->insert("productWeight > 100");
    delay->connectTo(decide);

    Dispose* department1 = plugins->newInstance<Dispose>(model);
    department1->setDescription("Department 1");
    Dispose* department2 = plugins->newInstance<Dispose>(model);
    department2->setDescription("Department 2");
    decide->connectTo(department1);
    decide->connectTo(department2);

    ModelSimulation* simulation = model->getSimulation();
    simulation->setNumberOfReplications(3);
    simulation->setReplicationLength(10, Util::TimeUnit::minute);
    simulation->setWarmUpPeriod(0.05, Util::TimeUnit::minute);
    model->save("./models/Smart_Expression.gen");
    model->getSimulation()->start();
    
    delete genesys;
    return 0;
};
