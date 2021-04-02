/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "FullSimulationOfComplexModel.h"

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
#include "Dummy.h"

// Model elements
#include "EntityType.h"
#include "Attribute.h"
#include "Variable.h"
#include "ProbDistribDefaultImpl1.h"
#include "EntityGroup.h"
#include "Formula.h"
#include "OLD_ODEelement.h"

FullSimulationOfComplexModel::FullSimulationOfComplexModel() {

}

/**
 * This is the main function of the application. 
 * It instanciates the simulator, builds a simulation model and then simulate that model.
 */
int FullSimulationOfComplexModel::main(int argc, char** argv) {
	Simulator* simulator = new Simulator();

	// insert "fake plugins" since plugins based on dynamic loaded library are not implemented yet
	this->insertFakePluginsByHand(simulator);

	// creates an empty model
	Model* model = new Model(simulator);

	// Handle traces and simulation events to output them
	TraceManager* tm = model->getTracer();
	this->setDefaultTraceHandlers(tm);

	// get easy access to classes used to insert components and elements into a model
	ComponentManager* components = model->getComponents();
	ElementManager* elements = model->getElements();

	//
	// build the simulation model
	//
	ModelInfo* infos = model->getInfos();
	infos->setAnalystName("Your name");
	infos->setProjectTitle("The title of the project");
	infos->setDescription("This simulation model tests the components and elements that have been implemented so far.");
	infos->setReplicationLength(1e4);
	infos->setReplicationLengthTimeUnit(Util::TimeUnit::minute);
	infos->setNumberOfReplications(3000);
	tm->setTraceLevel(Util::TraceLevel::modelResult);

	EntityType* entityType1 = new EntityType(model, "Representative_EntityType");
	elements->insert(entityType1);

	Create* create1 = new Create(model);
	create1->setEntityType(entityType1);
	create1->setTimeBetweenCreationsExpression("EXPO(5)");
	create1->setTimeUnit(Util::TimeUnit::minute);
	create1->setEntitiesPerCreation(1);
	components->insert(create1);

	Attribute* attribute1 = new Attribute(model, "Attribute_1");
	elements->insert(attribute1);
	Variable* variable1 = new Variable(model, "Variable_1");
	elements->insert(variable1);

	Assign* assign1 = new Assign(model);
	Assign::Assignment* attrib2Assignment = new Assign::Assignment("Variable_1", "Variable_1 + 1");
	assign1->getAssignments()->insert(attrib2Assignment);
	Assign::Assignment* attrib1Assignment = new Assign::Assignment("Attribute_1", "Variable_1");
	assign1->getAssignments()->insert(attrib1Assignment);
	components->insert(assign1);

	Decide* decide1 = new Decide(model);
	decide1->getConditions()->insert("UNIF(0,1)>0.5");

	Resource* maquina1 = new Resource(model, "Máquina 1");
	maquina1->setCapacity(1);
	elements->insert(maquina1);

	Queue* filaSeize1 = new Queue(model);
	filaSeize1->setOrderRule(Queue::OrderRule::FIFO);
	elements->insert(filaSeize1);

	Seize* seize1 = new Seize(model);
	seize1->setSeizeRequest(new ResourceItemRequest(maquina1));
	seize1->setQueue(filaSeize1);
	components->insert(seize1);

	Delay* delay1 = new Delay(model);
	delay1->setDelayExpression("NORM(5,3)");
	delay1->setDelayTimeUnit(Util::TimeUnit::minute);

	components->insert(delay1);

	Release* release1 = new Release(model);
	release1->setReleaseRequest(new ResourceItemRequest(maquina1));
	components->insert(release1);

	Record* record1 = new Record(model);
	record1->setExpressionName("Tempo total no sistema");
	record1->setExpression("TNOW - Entity.ArrivalTime");
	record1->setFilename("./temp/TotalTimeInSystem.txt");
	components->insert(record1);

	Dispose* dispose1 = new Dispose(model);
	components->insert(dispose1);

	// connect model components to create a "workflow" -- should always start from a SourceModelComponent and end at a SinkModelComponent (it will be checked)
	create1->getNextComponents()->insert(assign1);
	assign1->getNextComponents()->insert(decide1);
	decide1->getNextComponents()->insert(seize1);
	decide1->getNextComponents()->insert(dispose1);
	seize1->getNextComponents()->insert(delay1);
	delay1->getNextComponents()->insert(release1);
	release1->getNextComponents()->insert(record1);
	record1->getNextComponents()->insert(dispose1);

	// insert the model into the simulator
	simulator->getModels()->insert(model);

	// then save the model into a text file
	model->save("./temp/fullSimulationModel.txt");

	// execute the simulation
	model->getSimulation()->start();

	return 0;
};
