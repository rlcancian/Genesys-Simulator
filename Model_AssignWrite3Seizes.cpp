/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   FourthExampleOfSimulation.cpp
 * Author: rlcancian
 * 
 * Created on 24 de Setembro de 2019, 20:56
 */

#include "Model_AssignWrite3Seizes.h"

// GEnSyS Simulator
#include "Simulator.h"

// Model Components
#include "Create.h"
#include "Delay.h"
#include "Dispose.h"
#include "Seize.h"
#include "Release.h"
#include "Assign.h"
#include "Record.h"
#include "Decide.h"
#include "Write.h"

// Model elements
#include "ElementManager.h"
#include "EntityType.h"
#include "Attribute.h"
#include "Variable.h"
#include "ProbDistribDefaultImpl1.h"
#include "EntityGroup.h"
#include "Set.h"

Model_AssignWrite3Seizes::Model_AssignWrite3Seizes() {
}

int Model_AssignWrite3Seizes::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	genesys->getTracer()->setTraceLevel(Util::TraceLevel::L6_internal); //modelResult); //componentArrival);
	this->setDefaultTraceHandlers(genesys->getTracer());
	this->insertFakePluginsByHand(genesys);

	Model* model = genesys->getModels()->newModel();
	this->setDefaultEventHandlers(model->getOnEvents());

	// build the simulation model
	ModelSimulation* sim = model->getSimulation();
	sim->setNumberOfReplications(5);
	sim->setReplicationLength(100);
	EntityType* part = new EntityType(model, "Part");
	Create* create1 = new Create(model);
	create1->setEntityType(part);
	create1->setTimeBetweenCreationsExpression("norm(1.5,0.5)");
	create1->setTimeUnit(Util::TimeUnit::second);
	create1->setEntitiesPerCreation(1);
	Assign* assign1 = new Assign(model);
	assign1->getAssignments()->insert(new Assign::Assignment("varNextIndex", "varNextIndex + 1"));
	assign1->getAssignments()->insert(new Assign::Assignment("index", "varNextIndex"));
	Attribute* attr1 = new Attribute(model, "index");
	Variable* var1 = new Variable(model, "varNextIndex");
	Write* write1 = new Write(model);
	write1->setWriteToType(Write::WriteToType::SCREEN);
	write1->writeElements()->insert(new WriteElement("Atributo index: "));
	write1->writeElements()->insert(new WriteElement("index", true, true));
	write1->writeElements()->insert(new WriteElement("Variável nextIndex: "));
	write1->writeElements()->insert(new WriteElement("varNextIndex", true, true));
	write1->writeElements()->insert(new WriteElement("Quantidade ocupada das máquinas: "));
	write1->writeElements()->insert(new WriteElement("NR( Machine_1 )", true));
	write1->writeElements()->insert(new WriteElement(", "));
	write1->writeElements()->insert(new WriteElement("NR(Machine_2)", true));
	write1->writeElements()->insert(new WriteElement(", "));
	write1->writeElements()->insert(new WriteElement("NR(Machine_3)", true, true));
	write1->writeElements()->insert(new WriteElement("Estado das máquinas: "));
	write1->writeElements()->insert(new WriteElement("STATE(Machine_1)", true));
	write1->writeElements()->insert(new WriteElement(", "));
	write1->writeElements()->insert(new WriteElement("STATE(Machine_2)", true));
	write1->writeElements()->insert(new WriteElement(", "));
	write1->writeElements()->insert(new WriteElement("STATE(Machine_3)", true, true));
	write1->writeElements()->insert(new WriteElement("Quantidade de máquinas ocupadas no Set: "));
	write1->writeElements()->insert(new WriteElement("SETSUM(Machine_Set)", true, true));
	write1->writeElements()->insert(new WriteElement("Quantidade de entidades na fila 3: "));
	write1->writeElements()->insert(new WriteElement("NQ(Queue_Seize_3)", true, true));
	write1->writeElements()->insert(new WriteElement("Somatório do atributo 'index' das entidades na fila 3: "));
	write1->writeElements()->insert(new WriteElement("SAQUE(Queue_Seize_3,index)", true, true));
	write1->writeElements()->insert(new WriteElement("Valor do atributo 'index' da 2ª entidade na fila 3: "));
	write1->writeElements()->insert(new WriteElement("AQUE(Queue_Seize_3,2,index)", true, true));
	write1->writeElements()->insert(new WriteElement("Tempo médio das entidades na fila 3: "));
	write1->writeElements()->insert(new WriteElement("TAVG(Queue_Seize_3.TimeInQueue)", true, true));
	//
	Resource* machine1 = new Resource(model, "Machine_1");
	machine1->setCapacity(1);
	Resource* machine2 = new Resource(model, "Machine_2");
	machine2->setCapacity(2);
	Resource* machine3 = new Resource(model, "Machine_3");
	machine3->setCapacity(3);
	Set* machSet = new Set(model, "Machine_Set");
	machSet->setSetOfType(Util::TypeOf<Resource>());
	machSet->getElementSet()->insert(machine1);
	machSet->getElementSet()->insert(machine2);
	machSet->getElementSet()->insert(machine3);
	Decide* decide1 = new Decide(model);
	decide1->getConditions()->insert("NR(Machine_1) < MR(Machine_1)");
	decide1->getConditions()->insert("NR(Machine_2) < MR(Machine_2)");
	Queue* queueSeize1 = new Queue(model, "Queue_Seize_1");
	queueSeize1->setOrderRule(Queue::OrderRule::FIFO);
	Seize* seize1 = new Seize(model);
	seize1->getSeizeRequests()->insert(new SeizableItemRequest(machine1));
	seize1->setQueue(queueSeize1);
	Delay* delay1 = new Delay(model);
	delay1->setDelayExpression("norm(15,1)");
	delay1->setDelayTimeUnit(Util::TimeUnit::second);
	Release* release1 = new Release(model);
	release1->getReleaseRequests()->insert(new SeizableItemRequest(machine1));
	Queue* queueSeize2 = new Queue(model, "Queue_Seize_2");
	queueSeize2->setOrderRule(Queue::OrderRule::FIFO);
	Seize* seize2 = new Seize(model);
	seize2->getSeizeRequests()->insert(new SeizableItemRequest(machine2));
	seize2->setQueue(queueSeize2);
	Delay* delay2 = new Delay(model);
	delay2->setDelayExpression("norm(15,1)");
	delay2->setDelayTimeUnit(Util::TimeUnit::second);
	Release* release2 = new Release(model);
	release2->getReleaseRequests()->insert(new SeizableItemRequest(machine2));
	Queue* queueSeize3 = new Queue(model, "Queue_Seize_3");
	queueSeize3->setOrderRule(Queue::OrderRule::FIFO);
	Seize* seize3 = new Seize(model);
	seize3->getSeizeRequests()->insert(new SeizableItemRequest(machine3));
	seize3->setQueue(queueSeize3);
	Delay* delay3 = new Delay(model);
	delay3->setDelayExpression("norm(15,1)");
	delay3->setDelayTimeUnit(Util::TimeUnit::second);
	Release* release3 = new Release(model);
	release3->getReleaseRequests()->insert(new SeizableItemRequest(machine3));
	Dispose* dispose1 = new Dispose(model);
	//
	create1->getNextComponents()->insert(assign1);
	assign1->getNextComponents()->insert(write1);
	write1->getNextComponents()->insert(decide1);
	decide1->getNextComponents()->insert(seize1);
	decide1->getNextComponents()->insert(seize2);
	decide1->getNextComponents()->insert(seize3);
	seize1->getNextComponents()->insert(delay1);
	delay1->getNextComponents()->insert(release1);
	release1->getNextComponents()->insert(dispose1);
	seize2->getNextComponents()->insert(delay2);
	delay2->getNextComponents()->insert(release2);
	release2->getNextComponents()->insert(dispose1);
	seize3->getNextComponents()->insert(delay3);
	delay3->getNextComponents()->insert(release3);
	release3->getNextComponents()->insert(dispose1);
	//
	model->save("./models/Model_StatationRouteSequence.txt");
	sim->start();
	return 0;
}

