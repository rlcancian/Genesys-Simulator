/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "FullSimulationOfComplexModel.h"

// GEnSyS Simulator
#include "../../../../kernel/simulator/Simulator.h"
#include "../../../../kernel/simulator/ModelDataManager.h"

// Model Components
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/Delay.h"
#include "../../../../plugins/components/Dispose.h"
#include "../../../../plugins/components/Seize.h"
#include "../../../../plugins/components/Release.h"
#include "../../../../plugins/components/Assign.h"
#include "../../../../plugins/components/Record.h"
#include "../../../../plugins/components/Decide.h"

// Model data definitions
#include "../../../../kernel/simulator/Attribute.h"
#include "../../../../plugins/data/Variable.h"

FullSimulationOfComplexModel::FullSimulationOfComplexModel() {

}

/**
 * This is the main function of the application. 
 * It instanciates the simulator, builds a simulation model and then simulate that model.
 */
int FullSimulationOfComplexModel::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	// insert "fake plugins" since plugins based on dynamic loaded library are not implemented yet
	genesys->getPluginManager()->autoInsertPlugins("autoloadplugins.txt");
	// creates an empty model
	Model* model = genesys->getModelManager()->newModel();
	// Handle traces and simulation events to output them
	TraceManager* tm = model->getTracer();
	this->setDefaultTraceHandlers(tm);
	//
	PluginManager* plugins = genesys->getPluginManager();
	// get easy access to classes used to insert components and elements into a model
	ComponentManager* components = model->getComponentManager();
	//
	// build the simulation model
	//
	ModelInfo* infos = model->getInfos();
	infos->setAnalystName("Your name");
	infos->setProjectTitle("The title of the project");
	infos->setDescription("This simulation model tests the components and elements that have been implemented so far.");

	ModelSimulation* sim = model->getSimulation();
	sim->setReplicationLength(1e4);
	sim->setReplicationLengthTimeUnit(Util::TimeUnit::minute);
	sim->setNumberOfReplications(30);
	tm->setTraceLevel(TraceManager::Level::L2_results);

	//EntityType* entityType1 = plugins->newInstance<EntityType>(model, "Representative_EntityType");

	Create* create1 = plugins->newInstance<Create>(model);
	//create1->setEntityType(entityType1);
	create1->setTimeBetweenCreationsExpression("EXPO(5)", Util::TimeUnit::minute);
	create1->setEntitiesPerCreation(1);
	components->insert(create1);

	plugins->newInstance<Attribute>(model, "Attribute_1");
	plugins->newInstance<Variable>(model, "Variable_1");

	Assign* assign1 = plugins->newInstance<Assign>(model);
	Assignment* attrib2Assignment = new Assignment("Variable_1", "Variable_1 + 1");
	assign1->getAssignments()->insert(attrib2Assignment);
	Assignment* attrib1Assignment = new Assignment("Attribute_1", "Variable_1");
	assign1->getAssignments()->insert(attrib1Assignment);

	Decide* decide1 = plugins->newInstance<Decide>(model);
	decide1->getConditions()->insert("UNIF(0,1) > 0.5");

	Resource* maquina1 = plugins->newInstance<Resource>(model, "Máquina 1");
	maquina1->setCapacity(1);

	Queue* filaSeize1 = plugins->newInstance<Queue>(model);
	filaSeize1->setOrderRule(Queue::OrderRule::FIFO);

	Seize* seize1 = plugins->newInstance<Seize>(model);
	seize1->getSeizeRequests()->insert(new SeizableItem(maquina1));
	seize1->setQueue(filaSeize1);

	Delay* delay1 = plugins->newInstance<Delay>(model);
	delay1->setDelayExpression("NORM(5, 3)", Util::TimeUnit::minute);

	Release* release1 = plugins->newInstance<Release>(model);
	release1->getReleaseRequests()->insert(new SeizableItem(maquina1));

	Record* record1 = plugins->newInstance<Record>(model);
	record1->setExpressionName("Tempo total no sistema");
	record1->setExpression("TNOW - Entity.ArrivalTime");
	record1->setFilename("./temp/TotalTimeInSystem.gen");

	Dispose* dispose1 = plugins->newInstance<Dispose>(model);

	// connect model components to create a "workflow" -- should always start from a SourceModelComponent and end at a SinkModelComponent (it will be checked)
	create1->getConnectionManager()->insert(assign1);
	assign1->getConnectionManager()->insert(decide1);
	decide1->getConnectionManager()->insert(seize1);
	decide1->getConnectionManager()->insert(dispose1);
	seize1->getConnectionManager()->insert(delay1);
	delay1->getConnectionManager()->insert(release1);
	release1->getConnectionManager()->insert(record1);
	record1->getConnectionManager()->insert(dispose1);
	// then save the model into a text file
	model->save("./models/AssignWrite3Seizes.gen");
	// execute the simulation
	model->getSimulation()->start();
	delete genesys;
	return 0;
};
