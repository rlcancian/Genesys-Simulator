/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/class.cc to edit this template
 */

#include "Smart_AssignExample.h"
#include "../../../../kernel/simulator/Simulator.h"
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/Process.h"
#include "../../../../plugins/components/Dispose.h"
#include "../../../../plugins/components/Decide.h"
#include "../../../../plugins/components/Assign.h"
#include "../../../TraitsApp.h"

Smart_AssignExample::Smart_AssignExample() {
}

int Smart_AssignExample::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();
	// create model
	Create *create = plugins->newInstance<Create>(model);
    create->setName("Calls Arrive");
    create->setEntityTypeName("Call");
    create->setTimeBetweenCreationsExpression("expo(1)", Util::TimeUnit::hour);
    
    Assign *assign = plugins->newInstance<Assign>(model);
    assign->setName("Assign Module");
    // "DISC(0.5, 0, 1, 1)not implemented, using another distribution
    assign->getAssignments()->insert(new Assignment(model, "Entity.Type", "unif(2,4)", true));
    
    plugins->newInstance<Attribute>(model, "Entity.Type");
    
    Decide *decide = plugins->newInstance<Decide>(model);
    decide->setName("Root Call");
    decide->getConditions()->insert("Entity.Type < 3");
    
    Resource *resource1 = plugins->newInstance<Resource>(model, "Operator");
    resource1->setCapacity(1);
    
    Process *process1 = plugins->newInstance<Process>(model);
    process1->setName("Costume Service");
    process1->setPriority(2);
    process1->getSeizeRequests()->insert(new SeizableItem(resource1, "1"));
    process1->setQueueableItem(new QueueableItem(plugins->newInstance<Queue>(model)));
    process1->setDelayExpression("tria(0.5,1,1.5)");
    process1->setDelayTimeUnit(Util::TimeUnit::hour);

    Resource *resource2 = plugins->newInstance<Resource>(model, "Accountant");
    
    Process *process2 = plugins->newInstance<Process>(model);
    process2->setName("Accounting");
    process2->setPriority(2);
    process2->getSeizeRequests()->insert(new SeizableItem(resource2, "1"));
    process2->setQueueableItem(new QueueableItem(plugins->newInstance<Queue>(model)));
    process2->setDelayExpression("tria(0.5,1,1.5)");
    process2->setDelayTimeUnit(Util::TimeUnit::hour);
    
    Dispose *dispose = plugins->newInstance<Dispose>(model);
    dispose->setName("Calls Finished");

    create->getConnectionManager()->insert(assign);
    assign->getConnectionManager()->insert(decide);
    decide->getConnectionManager()->insert(process1);
    decide->getConnectionManager()->insert(process2);
    process1->getConnectionManager()->insert(dispose);
    process2->getConnectionManager()->insert(dispose);
    
    ModelSimulation *s = model->getSimulation();
    s->setNumberOfReplications(3);
    double replicationLength = 1;
    s->setReplicationLength(replicationLength, Util::TimeUnit::week);
    s->setWarmUpPeriod(replicationLength * 0.05);
    s->setWarmUpPeriodTimeUnit(Util::TimeUnit::week);
    model->save("./models/Smart_AssignExample.gen");
    s->start();
    
    delete genesys;
    return 0;
}
