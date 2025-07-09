/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/file.cc to edit this template
 */


#include "Smart_BatchAndSeparate.h"

#include "../../../../kernel/simulator/Simulator.h"
#include "../../../../kernel/simulator/Attribute.h"
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/Dispose.h"
#include "../../../../plugins/components/Assign.h"
#include "../../../../plugins/components/Batch.h"
#include "../../../../plugins/components/Delay.h"
#include "../../../../plugins/components/Separate.h"
#include "../../../TraitsApp.h"

Smart_BatchAndSeparate::Smart_BatchAndSeparate() {
}

int Smart_BatchAndSeparate::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();
	// create model
	Create* cr1 = plugins->newInstance<Create>(model);
    cr1->setEntityTypeName("Entity 1");
    cr1->setTimeBetweenCreationsExpression("expo(8)", Util::TimeUnit::minute);
    cr1->setEntitiesPerCreation(1);
    
	Create* cr2 = plugins->newInstance<Create>(model);
    cr2->setEntityTypeName("Entity 1");
    cr2->setTimeBetweenCreationsExpression("expo(8)", Util::TimeUnit::minute);
    cr2->setEntitiesPerCreation(1);
    
	Batch* ba1 = plugins->newInstance<Batch>(model);
    // permanente pelo modelo do arena, mas aqui é temporário;
    ba1->setBatchSize("5");
    ba1->setGroupedAttributes(Batch::GroupedAttribs::LastEntity);
    
	Batch* ba2 = plugins->newInstance<Batch>(model);
    ba2->setBatchSize("10");
    ba1->setGroupedAttributes(Batch::GroupedAttribs::LastEntity);
    
	Separate* se2 = plugins->newInstance<Separate>(model);
    
	Dispose* di1 = plugins->newInstance<Dispose>(model);
    
	Dispose* di2 = plugins->newInstance<Dispose>(model);

    cr1->getConnectionManager()->insert(ba1);
    ba1->getConnectionManager()->insert(di1);
    
    cr2->getConnectionManager()->insert(ba2);
    ba2->getConnectionManager()->insert(se2);
    se2->getConnectionManager()->insert(di2);
    
	ModelSimulation* s = model->getSimulation();
    s->setNumberOfReplications(300);
    double replicationLength = 2700;
    s->setReplicationLength(1000);
    s->setReplicationLength(replicationLength, Util::TimeUnit::minute);
    s->setWarmUpPeriod(replicationLength * 0.05);
    s->setWarmUpPeriodTimeUnit(Util::TimeUnit::minute);
	model->save("./models/Smart_BatchAndSeparate.gen");
    s->start();
    
    delete genesys;
    return 0;
}
