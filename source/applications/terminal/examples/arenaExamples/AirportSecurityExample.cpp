#include "AirportSecurityExample.h"
#include "../../../../kernel/simulator/Simulator.h"
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/Process.h"
#include "../../../../plugins/components/Decide.h"
#include "../../../../plugins/components/Dispose.h"

AirportSecurityExample::AirportSecurityExample() {
}

int AirportSecurityExample::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	this->setDefaultTraceHandlers(genesys->getTraceManager());
	genesys->getPluginManager()->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();
	PluginManager* plugins = genesys->getPluginManager();

	model->getInfos()->setName("Airport Security Example");

	Create* create = plugins->newInstance<Create>(model);
	create->setDescription("Passengers arrive to security");
	create->setEntityTypeName("Passenger");
	create->setTimeBetweenCreationsExpression("expo(2)");
	create->setTimeUnit(Util::TimeUnit::minute);

	Resource* officer = plugins->newInstance<Resource>(model, "Transportation Security Officer");

	Process* process = plugins->newInstance<Process>(model);
	process->setDescription("Check for proper identification");
	process->getSeizeRequests()->insert(new SeizableItem(officer));
	process->setQueueableItem(new QueueableItem(model, "Check for Proper Identification.Queue"));
	process->setDelayExpression("tria(0.75, 1.5, 3)");
	process->setDelayTimeUnit(Util::TimeUnit::minute);

	Decide* decide = plugins->newInstance<Decide>(model);
	decide->setDescription("Pass security?");
	decide->getConditions()->insert("unif(0, 1) < 0.96");

	Dispose* disposeCleared = plugins->newInstance<Dispose>(model);
	disposeCleared->setDescription("Cleared");
	Dispose* disposeDenied = plugins->newInstance<Dispose>(model);
	disposeDenied->setDescription("Denied");

	create->getConnections()->insert(process);
	process->getConnections()->insert(decide);
	decide->getConnections()->insert(disposeCleared);
	decide->getConnections()->insert(disposeDenied);

	genesys->getTraceManager()->setTraceLevel(TraceManager::Level::L5_event);
	model->getSimulation()->setReplicationLength(12, Util::TimeUnit::hour);
	model->getSimulation()->setNumberOfReplications(2);
	model->getSimulation()->setWarmUpPeriod(0.5, Util::TimeUnit::hour);

	model->getSimulation()->start();
	delete genesys;
	return 0;
}

