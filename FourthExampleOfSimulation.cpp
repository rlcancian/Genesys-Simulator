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

#include "FourthExampleOfSimulation.h"

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

FourthExampleOfSimulation::FourthExampleOfSimulation() {
}

int FourthExampleOfSimulation::main(int argc, char** argv) {
	Simulator* simulator = new Simulator();
	simulator->getTracer()->setTraceLevel(Util::TraceLevel::everythingMostDetailed); //modelResult); //componentArrival);
	this->setDefaultTraceHandlers(simulator->getTracer());
	this->insertFakePluginsByHand(simulator);
	bool wantToCreateNewModelAndSaveInsteadOfJustLoad = false; //true;
	Model* model;
	if (wantToCreateNewModelAndSaveInsteadOfJustLoad) {
		model = new Model(simulator);
		// build the simulation model
		ModelInfo* infos = model->getInfos();
		infos->setNumberOfReplications(5);
		infos->setReplicationLength(100);
		EntityType* part = new EntityType(model, "Part");
		// model->insert(part);
		Create* create1 = new Create(model);
		create1->setEntityType(part);
		create1->setTimeBetweenCreationsExpression("norm(1.5,0.5)");
		create1->setTimeUnit(Util::TimeUnit::second);
		create1->setEntitiesPerCreation(1);
		// model->insert(create1);
		Assign* assign1 = new Assign(model);
		assign1->assignments()->insert(new Assign::Assignment("varNextIndex", "varNextIndex + 1"));
		assign1->assignments()->insert(new Assign::Assignment("index", "varNextIndex"));
		// model->insert(assign1);
		Attribute* attr1 = new Attribute(model, "index");
		// model->insert(attr1);
		Variable* var1 = new Variable(model, "varNextIndex");
		// model->insert(var1);
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
		// model->insert(write1);
		//
		Resource* machine1 = new Resource(model, "Machine_1");
		machine1->setCapacity(1);
		// model->insert(machine1);
		Resource* machine2 = new Resource(model, "Machine_2");
		machine2->setCapacity(2);
		// model->insert(machine2);
		Resource* machine3 = new Resource(model, "Machine_3");
		machine3->setCapacity(3);
		// model->insert(machine3);
		Set* machSet = new Set(model, "Machine_Set");
		machSet->setSetOfType(Util::TypeOf<Resource>());
		machSet->getElementSet()->insert(machine1);
		machSet->getElementSet()->insert(machine2);
		machSet->getElementSet()->insert(machine3);
		// model->insert(machSet);
		Decide* decide1 = new Decide(model);
		decide1->getConditions()->insert("NR(Machine_1) < MR(Machine_1)");
		decide1->getConditions()->insert("NR(Machine_2) < MR(Machine_2)");
		// model->insert(decide1);
		Queue* queueSeize1 = new Queue(model, "Queue_Seize_1");
		queueSeize1->setOrderRule(Queue::OrderRule::FIFO);
		// model->insert(queueSeize1);
		Seize* seize1 = new Seize(model);
		seize1->setSeizeRequest(new ResourceItemRequest(machine1));
		seize1->setQueue(queueSeize1);
		// model->insert(seize1);
		Delay* delay1 = new Delay(model);
		delay1->setDelayExpression("norm(15,1)");
		delay1->setDelayTimeUnit(Util::TimeUnit::second);
		// model->insert(delay1);
		Release* release1 = new Release(model);
		release1->setReleaseRequest(new ResourceItemRequest(machine1));
		// model->insert(release1);
		Queue* queueSeize2 = new Queue(model, "Queue_Seize_2");
		queueSeize2->setOrderRule(Queue::OrderRule::FIFO);
		// model->insert(queueSeize2);
		Seize* seize2 = new Seize(model);
		seize2->setSeizeRequest(new ResourceItemRequest(machine2));
		seize2->setQueue(queueSeize2);
		// model->insert(seize2);
		Delay* delay2 = new Delay(model);
		delay2->setDelayExpression("norm(15,1)");
		delay2->setDelayTimeUnit(Util::TimeUnit::second);
		// model->insert(delay2);
		Release* release2 = new Release(model);
		release2->setReleaseRequest(new ResourceItemRequest(machine2));
		// model->insert(release2);
		Queue* queueSeize3 = new Queue(model, "Queue_Seize_3");
		queueSeize3->setOrderRule(Queue::OrderRule::FIFO);
		// model->insert(queueSeize3);
		Seize* seize3 = new Seize(model);
		seize3->setSeizeRequest(new ResourceItemRequest(machine3));
		seize3->setQueue(queueSeize3);
		// model->insert(seize3);
		Delay* delay3 = new Delay(model);
		delay3->setDelayExpression("norm(15,1)");
		delay3->setDelayTimeUnit(Util::TimeUnit::second);
		// model->insert(delay3);
		Release* release3 = new Release(model);
		release3->setReleaseRequest(new ResourceItemRequest(machine3));
		// model->insert(release3);
		Dispose* dispose1 = new Dispose(model);
		// model->insert(dispose1);
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
		simulator->getModels()->insert(model);
		model->save("./temp/forthExampleOfSimulation.txt");
	} else { // load previously saved model
		simulator->getModels()->loadModel("./temp/forthExampleOfSimulation.txt");
		model = simulator->getModels()->current();
	}
	this->setDefaultEventHandlers(model->getOnEvents());
	model->getSimulation()->start();
	return 0;
}

