#include "Smart_Failures.h"


// GEnSyS Simulator
#include "../../../../kernel/simulator/Simulator.h"

// Model Components
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/Seize.h"
#include "../../../../plugins/components/Delay.h"
#include "../../../../plugins/components/Release.h"
#include "../../../../plugins/components/Dispose.h"
#include "../../../../plugins/data/Failure.h"
#include "../../../TraitsApp.h"

Smart_Failures::Smart_Failures() {
}

/**
 * This is the main function of the application.
 * It instanciates the simulator, builds a simulation model and then simulate that model.
 */
int Smart_Failures::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();
	// create model
	Create* create1 = plugins->newInstance<Create>(model);
	//create1->setTimeBetweenCreationsExpression("100");
	//create1->setFirstCreation(100);
	Failure* failure1 = plugins->newInstance<Failure>(model, "failure by time");
	failure1->setUpTimeExpression("norm(50,2)");
	failure1->setDownTimeExpression("5");
	Failure* failure2 = plugins->newInstance<Failure>(model, "failure by count");
	failure2->setFailureType(Failure::FailureType::COUNT);
	failure2->setCountExpression("10");
	failure2->setDownTimeExpression("2");
	Resource* machine1 = plugins->newInstance<Resource>(model, "Machine_1");
	machine1->insertFailure(failure1);
	machine1->insertFailure(failure2);
	Queue* queueSeize1 = plugins->newInstance<Queue>(model, "Seize_1.Queue");
	Seize* seize1 = plugins->newInstance<Seize>(model);
	seize1->getSeizeRequests()->insert(new SeizableItem(machine1, "1"));
	seize1->setQueue(queueSeize1);
	Delay* delay1 = plugins->newInstance<Delay>(model);
	//delay1->setDelay(100);
	Release* release1 = plugins->newInstance<Release>(model);
	release1->getReleaseRequests()->insert(new SeizableItem(machine1, "1"));
	Dispose* dispose1 = plugins->newInstance<Dispose>(model);
	// connect model components to create a "workflow"
	create1->getConnectionManager()->insert(seize1);
	seize1->getConnectionManager()->insert(delay1);
	delay1->getConnectionManager()->insert(release1);
	release1->getConnectionManager()->insert(dispose1);
	// set options, save and simulate
	ModelSimulation* sim = model->getSimulation();
	sim->setReplicationLength(1000, Util::TimeUnit::second);
	model->save("./models/Smart_Failures.gen");
	// execute the simulation
	sim->start();
	delete genesys;
	return 0;
};
