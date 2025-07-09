/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Smart_ParallelProcessingOfEntities.cpp
 * Author: Luiz e Guilherme
 * 
 * Created on Dezembro de 2022
 */

#include "Smart_ParallelProcessingOfEntities.h"

// you have to included need libs

// GEnSyS Simulator
#include "../../../../kernel/simulator/Simulator.h"

// Model Components
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/Separate.h"
#include "../../../../plugins/components/Clone.h"
#include "../../../../plugins/components/Process.h"
#include "../../../../plugins/components/Batch.h"
#include "../../../../plugins/components/Dispose.h"
#include "../../../../plugins/components/Assign.h"
#include "../../../TraitsApp.h"

Smart_ParallelProcessingOfEntities::Smart_ParallelProcessingOfEntities() {
}

/**
 * This is the main function of the application. 
 * It instanciates the simulator, builds a simulation model and then simulate that model.
 */
int Smart_ParallelProcessingOfEntities::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();
	// create model

    Create* createEquipmentArrives = plugins->newInstance<Create>(model, "createEquipmentArrives");
    Clone * clone_1 = plugins->newInstance<Clone>(model, "separate_1");
    Clone * clone_2 = plugins->newInstance<Clone>(model, "separate_2");
    Process* processCleanEquipment = plugins->newInstance<Process>(model, "processCleanEquipment");
    Process* processTestEquipment = plugins->newInstance<Process>(model, "processTestEquipment");
    Process* processCertifyEquipment = plugins->newInstance<Process>(model, "processCertifyEquipment");
    Batch* batchCombinePaperwork = plugins->newInstance<Batch>(model, "batchCombinePaperwork");
    Dispose* disposeEquipmentPlacedInWarehouse = plugins->newInstance<Dispose>(model, "disposeEquipmentPlacedInWarehouse");
    Assign* entitySerialNumber = plugins->newInstance<Assign>(model, "Assign serial number");

    // Create - Equipament Arrives
    createEquipmentArrives->getConnectionManager()->insert(entitySerialNumber);
    createEquipmentArrives->setEntityTypeName("Equipment");
    createEquipmentArrives->setTimeBetweenCreationsExpression("NORM(8,9)");
    createEquipmentArrives->setTimeUnit(Util::TimeUnit::minute);
    createEquipmentArrives->setFirstCreation(0.0);
    createEquipmentArrives->setEntitiesPerCreation(1);

    // Create the SerialNumber
    Variable* serial = plugins->newInstance<Variable>(model, "Serial");
    serial->setInitialValue(0);
    // Attribute* serialNumber = plugins->newInstance<Attribute>(model, "SerialNumber");
    
    entitySerialNumber->getConnectionManager()->insert(clone_1);
	entitySerialNumber->getAssignments()->insert(new Assignment(model, "Serial", "Serial + 1", false));
	entitySerialNumber->getAssignments()->insert(new Assignment(model, "Entity.SerialNumber", "Serial", true));

    // Clone - Separate 1
    clone_1->getConnectionManager()->insert(processCleanEquipment);
    clone_1->getConnectionManager()->insert(clone_2);
    clone_1->setNumClonesExpression("1");
    clone_1->setReportStatistics(false);
    
    // Separate 2
    clone_2->getConnectionManager()->insert(processCertifyEquipment);
    clone_2->getConnectionManager()->insert(processTestEquipment);
    clone_2->setNumClonesExpression("1");
    clone_2->setReportStatistics(false);

    // Process - Clean Equipment
    processCleanEquipment->getConnectionManager()->insert(batchCombinePaperwork);
    processCleanEquipment->setDelayExpression("UNIF(10,20)");
    processCleanEquipment->setDelayTimeUnit(Util::TimeUnit::minute);

    Resource* clerk_1 = plugins->newInstance<Resource>(model);
    clerk_1->setName("clerk_1");

    processCleanEquipment->getSeizeRequests()->insert(new SeizableItem(clerk_1));
    processCleanEquipment->setQueueableItem(new QueueableItem(plugins->newInstance<Queue>(model, "processCleanEquipmentQueue")));

    // Process - Test Equipment
    processTestEquipment->getConnectionManager()->insert(batchCombinePaperwork);
    processTestEquipment->setDelayExpression("UNIF(5,15)");
    processTestEquipment->setDelayTimeUnit(Util::TimeUnit::minute);

    Resource* clerk_2 = plugins->newInstance<Resource>(model);
    clerk_2->setName("clerk_2");

    processTestEquipment->getSeizeRequests()->insert(new SeizableItem(clerk_2));
    processTestEquipment->setQueueableItem(new QueueableItem(plugins->newInstance<Queue>(model, "processTestEquipmentQueue")));

    // Process - Certify Equipment
    processCertifyEquipment->getConnectionManager()->insert(batchCombinePaperwork);
    processCertifyEquipment->setDelayExpression("UNIF(5,10)");
    processCertifyEquipment->setDelayTimeUnit(Util::TimeUnit::minute);

    Resource* clerk_3 = plugins->newInstance<Resource>(model);
    clerk_3->setName("clerk_3");

    processCertifyEquipment->getSeizeRequests()->insert(new SeizableItem(clerk_3));
    processCertifyEquipment->setQueueableItem(new QueueableItem(plugins->newInstance<Queue>(model, "processCertifyEquipmentQueue")));

    // Batch - Combine Paperwork
    batchCombinePaperwork->getConnectionManager()->insert(disposeEquipmentPlacedInWarehouse);
    batchCombinePaperwork->setBatchSize("3");
    batchCombinePaperwork->setRule(Batch::Rule::ByAttribute);
    batchCombinePaperwork->setGroupedAttributes(Batch::GroupedAttribs::LastEntity);
    batchCombinePaperwork->setAttributeName("Entity.SerialNumber");
        
    // set options, save and simulate
    auto replicationLength = 100; 
	model->getSimulation()->setReplicationLength(std::numeric_limits<double>::max(), Util::TimeUnit::week); // This is a "infinity" value.
    model->getSimulation()->setWarmUpPeriod(replicationLength * 0.05);
    model->getSimulation()->setWarmUpPeriodTimeUnit(Util::TimeUnit::minute);
    model->getSimulation()->setTerminatingCondition("COUNT(disposeEquipmentPlacedInWarehouse.CountNumberIn) > 1000");
    model->getSimulation()->setNumberOfReplications(3);

    model->getInfos()->setName("Parallel Processing of Entities");
    model->save("./models/Smart_ParallelProcessingOfEntities.gen"); 

    model->getSimulation()->start();
	for (int i = 0; i < 1e9; i++);
    
    delete genesys;
    return 0;
};

