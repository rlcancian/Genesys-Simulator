#include "Airport_Extended1.h"


// GEnSyS Simulator
#include "../../../../kernel/simulator/Simulator.h"

// Model Components
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/Seize.h"
#include "../../../../plugins/components/Delay.h"
#include "../../../../plugins/components/Release.h"
#include "../../../../plugins/components/Decide.h"
#include "../../../../plugins/components/Dispose.h"

Airport_Extended1::Airport_Extended1() {
}

/**
 * This is the main function of the application.
 * It instanciates the simulator, builds a simulation model and then simulate that model.
 */
int Airport_Extended1::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	this->setDefaultTraceHandlers(genesys->getTraceManager());
	genesys->getPluginManager()->autoInsertPlugins("autoloadplugins.txt");
	// create model
	Model* model = genesys->getModelManager()->newModel();
	PluginManager* plugins = genesys->getPluginManager();
        
        //Inicio do modelo
        Create* create1 = plugins->newInstance<Create>(model);
        create1->setEntityTypeName("Customer");
        create1->setTimeBetweenCreationsExpression("expo(2)");
        create1->setTimeUnit(Util::TimeUnit::minute);
        
        Resource* officer = plugins->newInstance<Resource>(model, "Transportation Security Officer");
        officer->setCapacity(1);
        officer->setCostBusyTimeUnit(12);
        officer->setCostIdleTimeUnit(12);
        Queue* queueOfficer = plugins->newInstance<Queue>(model, "Fila_Officer");
        Seize* seizeOfficer = plugins->newInstance<Seize>(model);
        seizeOfficer->setQueue(queueOfficer);
        seizeOfficer->getSeizeRequests()->insert(new SeizableItem(officer, "1"));
        Delay* delayOfficer = plugins->newInstance<Delay>(model);
        delayOfficer->setDelayExpression("TRIA(0.75, 1.5, 3)");
        delayOfficer->setDelayTimeUnit(Util::TimeUnit::minute);
        Release* releaseOfficer = plugins->newInstance<Release>(model);
        releaseOfficer->getReleaseRequests()->insert(new SeizableItem(officer, "1"));
        
        create1->getConnectionManager()->insert(seizeOfficer);
        seizeOfficer->getConnectionManager()->insert(delayOfficer);
        delayOfficer->getConnectionManager()->insert(releaseOfficer);
        
        Decide* decide1 = plugins->newInstance<Decide>(model);
	decide1->getConditions()->insert("UNIF(0,1) > 0.04");
        
        releaseOfficer->getConnectionManager()->insert(decide1);
        
        Resource* xray = plugins->newInstance<Resource>(model, "X-Ray Machine");
        xray->setCapacity(2);
        Queue* queueXray = plugins->newInstance<Queue>(model, "Fila_X-ray");
        Seize* seizeXray = plugins->newInstance<Seize>(model);
        seizeXray->setQueue(queueXray);
        seizeXray->getSeizeRequests()->insert(new SeizableItem(xray, "1"));
        Delay* delayXray = plugins->newInstance<Delay>(model);
        delayXray->setDelayExpression("TRIA(1.5, 2.5, 7)");
        delayXray->setDelayTimeUnit(Util::TimeUnit::minute);
        Release* releaseXray = plugins->newInstance<Release>(model);
        releaseXray->getReleaseRequests()->insert(new SeizableItem(xray, "1"));
        
        decide1->getConnectionManager()->insert(seizeXray);
        seizeXray->getConnectionManager()->insert(delayXray);
        delayXray->getConnectionManager()->insert(releaseXray);
        
        Dispose* dispose1 = plugins->newInstance<Dispose>(model, "Cleared");
        
        releaseXray->getConnectionManager()->insert(dispose1);
        
        Dispose* dispose2 = plugins->newInstance<Dispose>(model, "Denied");
        
        decide1->getConnectionManager()->insert(dispose2);
        
	// set options, save and simulate
	model->getSimulation()->setReplicationLength(10, Util::TimeUnit::hour);
        model->getSimulation()->setWarmUpPeriod(30);
        model->getSimulation()->setWarmUpPeriodTimeUnit(Util::TimeUnit::minute);
        model->getSimulation()->setNumberOfReplications(50);
	model->save("./models/Airport_Extended1.gen");
	model->getSimulation()->start();
	delete genesys;
	return 0;
};
