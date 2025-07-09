/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/class.cc to edit this template
 */

/* 
 * File:   Smart_BatchSeparate.cpp
 * Author: rlcancian
 * 
 * Created on 15 de janeiro de 2022, 09:51
 */

#include "Smart_BatchSeparate.h"

#include "../../../../kernel/simulator/Simulator.h"
#include "../../../../kernel/simulator/Attribute.h"
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/Dispose.h"
#include "../../../../plugins/components/Assign.h"
#include "../../../../plugins/components/Batch.h"
#include "../../../../plugins/components/Delay.h"
#include "../../../../plugins/components/Separate.h"
#include "../../../TraitsApp.h"

Smart_BatchSeparate::Smart_BatchSeparate() {
}

int Smart_BatchSeparate::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();
	// create model
	Create* cr = plugins->newInstance<Create>(model);
	cr->setEntityTypeName("normal");
	cr->setMaxCreations(50);
	Assign* as = plugins->newInstance<Assign>(model);
	as->getAssignments()->insert(new Assignment(model, "batchNum", "trunc(unif(0,5))"));
	Batch* ba = plugins->newInstance<Batch>(model);
	ba->setBatchSize("4");
	ba->setRule(Batch::Rule::ByAttribute);
	ba->setAttributeName("batchNum");
	ba->setGroupedAttributes(Batch::GroupedAttribs::SumAttributes);
	ba->setGroupedEntityTypeName("grouped");
	Delay* de = plugins->newInstance<Delay>(model);
	de->setDelay(25);
	Separate* se = plugins->newInstance<Separate>(model);
	Dispose* di = plugins->newInstance<Dispose>(model);
	// connect model components to create a "workflow"
	cr->getConnectionManager()->insert(as);
	as->getConnectionManager()->insert(ba);
	ba->getConnectionManager()->insert(de);
	de->getConnectionManager()->insert(se);
	se->getConnectionManager()->insert(di);
	// save, trace specific modules and aimulate
	ModelSimulation* sim = model->getSimulation();
	sim->setReplicationLength(100);
	sim->setShowReportsAfterReplication(false);
	sim->setShowReportsAfterSimulation(false);
	model->getTracer()->setTraceLevel(TraceManager::Level::L7_internal);
	model->getTracer()->setTraceSimulationRuleAllAllowed(false);
	model->getTracer()->addTraceSimulationExceptionRuleModelData(ba);
	model->getTracer()->addTraceSimulationExceptionRuleModelData(se);
	model->getTracer()->addTraceSimulationExceptionRuleModelData(sim);
	model->save("./models/Smart_BatchSeparate.gen");
	sim->start();
	delete genesys;
	return 0;
}
