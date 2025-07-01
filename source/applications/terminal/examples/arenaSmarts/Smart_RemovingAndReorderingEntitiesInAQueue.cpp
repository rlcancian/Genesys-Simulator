/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "Smart_RemovingAndReorderingEntitiesInAQueue.h"

// you have to included need libs

// GEnSyS Simulator
#include "../../../../kernel/simulator/Simulator.h"

// Model Components
#include "../../../../plugins/components/Assign.h"
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/Dispose.h"
#include "../../../../plugins/components/Process.h"
#include "../../../../plugins/components/Remove.h"
#include "../../../../plugins/components/Search.h"


#include "../../../../kernel/simulator/Attribute.h"
#include "../../../../plugins/data/Queue.h"
#include "../../../../plugins/data/Resource.h"
#include "../../../TraitsApp.h"

Smart_RemovingAndReorderingEntitiesInAQueue::Smart_RemovingAndReorderingEntitiesInAQueue() {
}

/**
 * This is the main function of the application. 
 * It instanciates the simulator, builds a simulation model and then simulate that model.
 */
int Smart_RemovingAndReorderingEntitiesInAQueue::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();
	// create model

	plugins->newInstance<Attribute>(model, "Wait_Time");
	plugins->newInstance<Attribute>(model, "j");
	Queue* queue1 = plugins->newInstance<Queue>(model, "queue1");
	Resource* machine = plugins->newInstance<Resource>(model, "Machine");
	Resource* machine2 = plugins->newInstance<Resource>(model, "Machine_2");

	Create* entityCreate = plugins->newInstance<Create>(model, "Entity_Create");
	entityCreate->setTimeBetweenCreationsExpression("tria(6,7,8)", Util::TimeUnit::minute);
	entityCreate->setMaxCreations(200);
	Assign* markTheTimeBeforeItEntersTheQueue = plugins->newInstance<Assign>(model, "Mark_the_time_before_it_enters_the_queue");
	markTheTimeBeforeItEntersTheQueue->getAssignments()->insert(new Assignment("Wait_Time", "TNOW"));
	Process* processingOnMachine = plugins->newInstance<Process>(model, "Processing_On_Machine");
	processingOnMachine->getSeizeRequests()->insert(new SeizableItem(machine, "1"));
	processingOnMachine->setQueueableItem(new QueueableItem(queue1));
	processingOnMachine->setDelayExpression("unif(7,10)", Util::TimeUnit::minute);
	Process* secondaryProcess = plugins->newInstance<Process>(model, "Secondary_Process");
	secondaryProcess->getSeizeRequests()->insert(new SeizableItem(machine2, "1"));
	secondaryProcess->setQueueableItem(new QueueableItem(plugins->newInstance<Queue>(model)));
	secondaryProcess->setDelayExpression("unif(6,9)", Util::TimeUnit::hour);
	Dispose* disposeEntity = plugins->newInstance<Dispose>(model, "Dispose_Entity");

	Create* createControlEntity = plugins->newInstance<Create>(model, "Create_Control_Entity");
	createControlEntity->setTimeBetweenCreationsExpression("1", Util::TimeUnit::minute);
	Assign* assign1 = plugins->newInstance<Assign>(model, "assign1"); //assign não existente no modelo do arena devido ao j ser um atributo reservado
	assign1->getAssignments()->insert(new Assignment("j", "0"));
	Search* searchQueueForEntitiesWaitingMoreThan20TimeUnits = plugins->newInstance<Search>(model, "Search_Queue_for_entities_waiting_more_than_20_time_units");
	searchQueueForEntitiesWaitingMoreThan20TimeUnits->setSearchIn(queue1);
	searchQueueForEntitiesWaitingMoreThan20TimeUnits->setSearchCondition("( TNOW - 20 ) > Wait_Time");
	searchQueueForEntitiesWaitingMoreThan20TimeUnits->setStartRank("0");
	searchQueueForEntitiesWaitingMoreThan20TimeUnits->setEndRank("NQ(queue1)");
	searchQueueForEntitiesWaitingMoreThan20TimeUnits->setSaveFounRankAttribute("j");
	Remove* removeEntityIfInQueueLongerThan20TimeUnits = plugins->newInstance<Remove>(model, "Remove_entity_if_in_queue_longer_than_20_time_units");
	removeEntityIfInQueueLongerThan20TimeUnits->setRemoveFrom(queue1);
	removeEntityIfInQueueLongerThan20TimeUnits->setRemoveStartRank("j");
	removeEntityIfInQueueLongerThan20TimeUnits->setRemoveEndRank("j");
	Assign* reassignPicturePriorityAndAttributeWaitTime = plugins->newInstance<Assign>(model, "Reassign_Picture_Priority_and_Attribute_Wait_Time");
	reassignPicturePriorityAndAttributeWaitTime->getAssignments()->insert(new Assignment("Priority", "1"));
	reassignPicturePriorityAndAttributeWaitTime->getAssignments()->insert(new Assignment("Wait_Time", "TNOW"));

	Dispose* disposeOfControlEntity = plugins->newInstance<Dispose>(model, "Dispose_of_Control_Entity");

	// connect model components to create a "workflow"
	entityCreate->getConnections()->insert(markTheTimeBeforeItEntersTheQueue);
	markTheTimeBeforeItEntersTheQueue->getConnections()->insert(processingOnMachine);
	processingOnMachine->getConnections()->insert(secondaryProcess);
	secondaryProcess->getConnections()->insert(disposeEntity);

	createControlEntity->getConnections()->insert(assign1);
	assign1->getConnections()->insert(searchQueueForEntitiesWaitingMoreThan20TimeUnits);
	searchQueueForEntitiesWaitingMoreThan20TimeUnits->getConnections()->insert(removeEntityIfInQueueLongerThan20TimeUnits);
	searchQueueForEntitiesWaitingMoreThan20TimeUnits->getConnections()->insert(disposeOfControlEntity);
	removeEntityIfInQueueLongerThan20TimeUnits->getConnections()->insert(disposeOfControlEntity);
	removeEntityIfInQueueLongerThan20TimeUnits->getConnections()->insert(reassignPicturePriorityAndAttributeWaitTime);
	reassignPicturePriorityAndAttributeWaitTime->getConnections()->insert(processingOnMachine);

	// set options, save and simulate
	model->getSimulation()->setReplicationLength(8, Util::TimeUnit::hour);
	model->getSimulation()->setReplicationReportBaseTimeUnit(Util::TimeUnit::minute);
	model->getSimulation()->setWarmUpPeriod(8*0.05, Util::TimeUnit::hour);
	model->getSimulation()->setNumberOfReplications(3);

	model->save("./models/Smart_RemovingAndReorderingEntitiesInAQueue.gen");
	model->getSimulation()->start();
	delete genesys;
	return 0;
};

