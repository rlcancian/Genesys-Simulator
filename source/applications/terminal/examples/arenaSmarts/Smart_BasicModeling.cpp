/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/class.cc to edit this template
 */

#include "Smart_BasicModeling.h"
#include "../../../../kernel/simulator/Simulator.h"
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/Process.h"
#include "../../../../plugins/components/Dispose.h"
#include "../../../TraitsApp.h"

Smart_BasicModeling::Smart_BasicModeling() {
}

int Smart_BasicModeling::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();
	// create model
	Create *create = plugins->newInstance<Create>(model);
    create->setName("Create 1");
    create->setEntityTypeName("Entity 1");
    create->setTimeBetweenCreationsExpression("expo(1)", Util::TimeUnit::hour);
    create->setEntitiesPerCreation(1);
    
    Process *process = plugins->newInstance<Process>(model);
    process->setName("Process 1");
    process->getSeizeRequests()->insert(new SeizableItem(plugins->newInstance<Resource>(model)));
    process->setQueueableItem(new QueueableItem(plugins->newInstance<Queue>(model)));
    process->setDelayExpression("tria(0.5,1,1.5)");
    process->setDelayTimeUnit(Util::TimeUnit::hour);

    Dispose *dispose = plugins->newInstance<Dispose>(model);

    create->getConnectionManager()->insert(process);
    process->getConnectionManager()->insert(dispose);

    ModelSimulation *s = model->getSimulation();
    s->setNumberOfReplications(300);
    double replicationLength = 7;
    s->setReplicationLength(replicationLength, Util::TimeUnit::week);
    s->setWarmUpPeriod(replicationLength * 0.05);
    s->setWarmUpPeriodTimeUnit(Util::TimeUnit::hour);
    model->save("./models/Smart_BasicModeling.gen");
    s->start();
    
    delete genesys;
    return 0;
}
