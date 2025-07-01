/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Smart_DecideNWayByChance.cpp
 * Author: guifrrs and LuizValdiero
 * 
 * Created on Dezembro de 2022
 */

#include "./Smart_DecideNWayByChance.h"

// Including the GEnSys Simulator
#include "../../../../kernel/simulator/Simulator.h"

// Components for the model
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/Delay.h"
#include "../../../../plugins/components/Decide.h"
#include "../../../../plugins/components/Dispose.h"
#include "../../../TraitsApp.h"

Smart_DecideNWayByChance::Smart_DecideNWayByChance() {

}

/**
 * This is the main function of the application. 
 * It create a new simulator instance, 
 * builds a simulation model and 
 * then simulate that model.
*/

int Smart_DecideNWayByChance::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();
	// create model
	Create* create_1 = plugins->newInstance<Create>(model, "Create_1");
    Delay* process_1 = plugins->newInstance<Delay>(model, "Process_1");
    Decide* decide_1 = plugins->newInstance<Decide>(model, "5_Way_Decide_By_Chance");
    Dispose* dispose_1 = plugins->newInstance<Dispose>(model, "Send_to_Warehouse");
    Dispose* dispose_2 = plugins->newInstance<Dispose>(model, "Send_to_Rebuild");
    Dispose* dispose_3 = plugins->newInstance<Dispose>(model, "Send_to_Refinish");
    Dispose* dispose_4 = plugins->newInstance<Dispose>(model, "Send_to_Repackage");
    Dispose* dispose_5 = plugins->newInstance<Dispose>(model, "Scrap");
    
    // Create 1
    create_1->getConnections()->insert(process_1);
    create_1->setEntityTypeName("Entity 1");
    create_1->setTimeBetweenCreationsExpression("EXPO(9)");
    create_1->setTimeUnit(Util::TimeUnit::minute);

    // Process 1
    // As the Process component on Genesys is a abstraction between Seize/Delay/Release
    // and Process on this model only use the delay, we switch for a Delay component.
    process_1->getConnections()->insert(decide_1);
    process_1->setDelayExpression("TRIA(8, 10, 12)");
    process_1->setDelayTimeUnit(Util::TimeUnit::minute);

    // 5 way Decide By Chance
    decide_1->getConnections()->insert(dispose_1);
    decide_1->getConditions()->insert("UNIF(0, 1) < 0.5");

    decide_1->getConnections()->insert(dispose_2);
    decide_1->getConditions()->insert("UNIF(0, 1) < 0.1");

    decide_1->getConnections()->insert(dispose_3);
    decide_1->getConditions()->insert("UNIF(0, 1) < 0.1");

    decide_1->getConnections()->insert(dispose_4);
    decide_1->getConditions()->insert("UNIF(0, 1) < 0.1");

    decide_1->getConnections()->insert(dispose_5);


    // Set options, save and run simulation.
    model->getInfos()->setName("Decide N-way By Chance");
	model->save("./models/Smart_DecideNWayByChance.gen");

	model->getSimulation()->setReplicationLength(std::numeric_limits<double>::max(), Util::TimeUnit::week); // This is a "infinity" value.
    model->getSimulation()->setReplicationReportBaseTimeUnit(Util::TimeUnit::minute);

    // Warmup should be 5% of replication length
    auto replicationLength = 400; 
    model->getSimulation()->setWarmUpPeriod(replicationLength * 0.05);
    model->getSimulation()->setWarmUpPeriodTimeUnit(Util::TimeUnit::minute);

    // This sums up all of our Dipose components to ensure that 1k entities are leaving the SMART.
    std::string countWarehouse = "COUNT(Send_to_Warehouse.CountNumberIn) + ";
    std::string countRebuild = "COUNT(Send_to_Rebuild.CountNumberIn) + ";
    std::string countRefinish = "COUNT(Send_to_Refinish.CountNumberIn) + ";
    std::string countRepackage = "COUNT(Send_to_Repackage.CountNumberIn) + ";
    std::string countScrap = "COUNT(Scrap.CountNumberIn)";
    std::string totalEntities = " > 1000";
    model->getSimulation()->setTerminatingCondition(countWarehouse + countRebuild + countRefinish + countRepackage + countScrap + totalEntities);
    model->getSimulation()->setNumberOfReplications(3);
    model->getSimulation()->start();

	for (int i = 0; i < 1e9; i++); // Give time to UI print everything.
    delete genesys;
    return 0;
}
