/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/class.cc to edit this template
 */

/* 
 * File:   Smart_AnElectronicAssemblyAndTestSystem.cpp
 * Author: rlcancian
 * 
 * Created on 6 de dezembro de 2021, 11:09
 */


#include "AnElectronicAssemblyAndTestSystem.h"
// GEnSyS Simulator
#include "../../../../kernel/simulator/Simulator.h"
// Model Components
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/Delay.h"
#include "../../../../plugins/components/Dispose.h"
#include "../../../../plugins/components/Assign.h"
#include "../../../../plugins/components/Record.h"
#include "../../../../plugins/components/Seize.h"
#include "../../../../plugins/components/Release.h"
#include "../../../../plugins/components/Decide.h"
// Model data definitions
#include "../../../../kernel/simulator/Attribute.h"
#include "../../../../plugins/components/SeizableItem.h"
#include "../../../../plugins/components/QueueableItem.h"

AnElectronicAssemblyAndTestSystem::AnElectronicAssemblyAndTestSystem() {
}

int AnElectronicAssemblyAndTestSystem::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	this->setDefaultTraceHandlers(genesys->getTraceManager());
	genesys->getPluginManager()->autoInsertPlugins("autoloadplugins.txt");
	// creating the model
	Model* model = genesys->getModelManager()->newModel();
	PluginManager* plugins = genesys->getPluginManager();
	ModelInfo* infos = model->getInfos();
	infos->setName("An Electronic Assembly And Test System");
	infos->setDescription("Example 4.1 from chapter 4 \"Modeling Basic Operations and Inputs\" of the Book \"...\"");
	model->getTracer()->setTraceLevel(TraceManager::Level::L9_mostDetailed);
	Create* create1 = plugins->newInstance<Create>(model, "Part A Arrive");
	Create* create2 = plugins->newInstance<Create>(model, "Part B Arrive");
	Assign* assign1 = plugins->newInstance<Assign>(model, "Assign Part A Sealer and Arrivel Time");
	Assign* assign2 = plugins->newInstance<Assign>(model, "Assign Part B Sealer and Arrivel Time");
	Seize* seize1 = plugins->newInstance<Seize>(model, "Prep A Process Seize");
	Delay* delay1 = plugins->newInstance<Delay>(model, "Prep A Process Delay");
	Release* release1 = plugins->newInstance<Release>(model, "Prep A Process Relase");
	Seize* seize2 = plugins->newInstance<Seize>(model, "Prep B Process Seize");
	Delay* delay2 = plugins->newInstance<Delay>(model, "Prep B Process Delay");
	Release* release2 = plugins->newInstance<Release>(model, "Prep B Process Releese");
	Seize* seize3 = plugins->newInstance<Seize>(model, "Sealer Seize");
	Delay* delay3 = plugins->newInstance<Delay>(model, "Sealer Delay");
	Release* release3 = plugins->newInstance<Release>(model, "Sealer Release");
	Seize* seize4 = plugins->newInstance<Seize>(model, "Rework Seize");
	Delay* delay4 = plugins->newInstance<Delay>(model, "Rework Delay");
	Release* release4 = plugins->newInstance<Release>(model, "Rework Release");
	Decide* decide1 = plugins->newInstance<Decide>(model, "Failed Sealer Inspector");
	Decide* decide2 = plugins->newInstance<Decide>(model, "Failed Rework Inspection");
	Record* record1 = plugins->newInstance<Record>(model, "Record Screpped Parts");
	Record* record2 = plugins->newInstance<Record>(model, "Record Salvaged Parts");
	Record* record3 = plugins->newInstance<Record>(model, "Recorded Shipped Parts");
	Dispose* dispose1 = plugins->newInstance<Dispose>(model, "Scrapped");
	Dispose* dispose2 = plugins->newInstance<Dispose>(model, "Salvaged");
	Dispose* dispose3 = plugins->newInstance<Dispose>(model, "Shipped");
	// connecting the components
	create1->getConnectionManager()->insert(assign1);
	assign1->getConnectionManager()->insert(seize1);
	seize1->getConnectionManager()->insert(delay1);
	delay1->getConnectionManager()->insert(release1);
	release1->getConnectionManager()->insert(seize3);
	seize3->getConnectionManager()->insert(delay3);
	delay3->getConnectionManager()->insert(release3);
	release3->getConnectionManager()->insert(decide1);
	create2->getConnectionManager()->insert(assign2);
	assign2->getConnectionManager()->insert(seize2);
	seize2->getConnectionManager()->insert(delay2);
	delay2->getConnectionManager()->insert(release2);
	release2->getConnectionManager()->insert(seize3);
	decide1->getConnectionManager()->insert(seize4);
	seize4->getConnectionManager()->insert(delay4);
	delay4->getConnectionManager()->insert(release4);
	release4->getConnectionManager()->insert(decide2);
	decide2->getConnectionManager()->insert(record1);
	record1->getConnectionManager()->insert(dispose1);
	decide2->getConnectionManager()->insert(record2);
	record2->getConnectionManager()->insert(dispose2);
	decide1->getConnectionManager()->insert(record3);
	record3->getConnectionManager()->insert(dispose3);
	// filling component settings
	create1->setEntityTypeName("Part_A");
	create1->setTimeBetweenCreationsExpression("expo(5)", Util::TimeUnit::minute);
	create2->setEntityTypeName("Part_B");
	create2->setTimeBetweenCreationsExpression("expo(30)");
	create2->setTimeUnit(Util::TimeUnit::minute);
	plugins->newInstance<Attribute>(model, "Sealer_Time");
	plugins->newInstance<Attribute>(model, "Arrive_Time");
	assign1->getAssignments()->insert(new Assignment("Sealer_Time", "tria(1,3,4)"));
	assign1->getAssignments()->insert(new Assignment("Arrive_Time", "tnow"));
	assign2->getAssignments()->insert(new Assignment("Sealer_Time", "weib(2.5, 5.3)"));
	assign2->getAssignments()->insert(new Assignment("Arrive_Time", "tnow"));
	Resource* prepA = plugins->newInstance<Resource>(model, "PrepA");
	seize1->getSeizeRequests()->insert(new SeizableItem(prepA, "1"));
	seize1->setQueueableItem(new QueueableItem(model, "QueuePrepA"));
	delay1->setDelayExpression("tria(1,4,8)");
	delay1->setDelayTimeUnit(Util::TimeUnit::minute);
	release1->getReleaseRequests()->insert(new SeizableItem(prepA, "1"));
	Resource* prepB = plugins->newInstance<Resource>(model, "PrepB");
	seize2->getSeizeRequests()->insert(new SeizableItem(prepB, "1"));
	seize2->setQueueableItem(new QueueableItem(model, "QueuePrepB"));
	delay2->setDelayExpression("tria(3,5,10)", Util::TimeUnit::minute);
	release2->getReleaseRequests()->insert(new SeizableItem(prepB, "1"));
	Resource* sealer = plugins->newInstance<Resource>(model, "Sealer");
	seize3->getSeizeRequests()->insert(new SeizableItem(sealer, "1"));
	seize3->setQueueableItem(new QueueableItem(model, "QueueSealer"));
	delay3->setDelayExpression("Sealer_Time", Util::TimeUnit::minute);
	release3->getReleaseRequests()->insert(new SeizableItem(sealer, "1"));
	decide1->getConditions()->insert("unif(0,1)<0.09");
	Resource* rework = plugins->newInstance<Resource>(model, "Rework");
	seize4->getSeizeRequests()->insert(new SeizableItem(rework, "1"));
	seize4->setQueueableItem(new QueueableItem(model, "QueueRework"));
	delay4->setDelayExpression("45");
	delay4->setDelayTimeUnit(Util::TimeUnit::minute);
	release4->getReleaseRequests()->insert(new SeizableItem(rework, "1"));
	decide2->getConditions()->insert("unif(0,1)<0.20");
	record1->setExpression("tnow - Arrive_Time");
	record1->setExpressionName("Scrapped Parts Total Time");
	record2->setExpression("tnow - Arrive_Time");
	record2->setExpressionName("Salvaged Parts Total Time");
	record3->setExpression("tnow - Arrive_Time");
	record3->setExpressionName("Shipped Parts Total Time");
	//setting model information
	model->getInfos()->setName("An Electronic Assembly and Test System");
	model->getInfos()->setDescription("B14 Chap 04 Modeling Basic Operations ans Inputs. Example section 4.1");
	//setting simulation parameters
	ModelSimulation* sim = model->getSimulation();
	sim->setReplicationLength(8);
	sim->setReplicationLengthTimeUnit(Util::TimeUnit::hour);
	sim->setWarmUpPeriod(1);
	sim->setWarmUpPeriodTimeUnit(Util::TimeUnit::hour);
	sim->setReplicationReportBaseTimeUnit(Util::TimeUnit::minute);
	//save the model
	model->save("./models/AnElectronicAssemblyAndTestSystem.gen");
	//simulating the model
	sim->start();
	delete genesys;
	return 0;
}
