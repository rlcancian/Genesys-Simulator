#include "AirportSecurityExampleExtended.h"
#include "../../../../kernel/simulator/Simulator.h"
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/Process.h"
#include "../../../../plugins/components/Decide.h"
#include "../../../../plugins/components/Dispose.h"
#include "../../../../plugins/components/Assign.h"
#include "../../../../plugins/components/Record.h"
#include "../../../../plugins/data/Variable.h"
#include "../../../../kernel/simulator/Attribute.h"

AirportSecurityExampleExtended::AirportSecurityExampleExtended() {
}

int AirportSecurityExampleExtended::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	this->setDefaultTraceHandlers(genesys->getTraceManager());
	genesys->getPluginManager()->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();
	PluginManager* plugins = genesys->getPluginManager();

	model->getInfos()->setName("Airport Security Example Extended");

	// Don't know if it's necessary to explicit these declarations.    
	plugins->newInstance<Attribute>(model, "a_Time_in");
	plugins->newInstance<Variable>(model, "v_Counter");

	Create* create = plugins->newInstance<Create>(model);
	create->setDescription("Passengers arrive to security");
	create->setEntityTypeName("Passenger");
	create->setTimeBetweenCreationsExpression("expo(2)");
	create->setTimeUnit(Util::TimeUnit::minute);

	Resource* officer = plugins->newInstance<Resource>(model, "Officer");
	officer->setCostBusyTimeUnit(12);
	officer->setCostIdleTimeUnit(12);

	Resource* xray = plugins->newInstance<Resource>(model, "XRay Machine");
	xray->setCapacity(2);

	Assign* assignTimeIn = plugins->newInstance<Assign>(model);
	assignTimeIn->getAssignments()->insert(new Assignment(model, "a_Time_in", "tnow", true));

	Process* processIdentification = plugins->newInstance<Process>(model);
	processIdentification->setDescription("Check for proper identification");
	processIdentification->getSeizeRequests()->insert(new SeizableItem(officer, "1"));
	processIdentification->setQueueableItem(new QueueableItem(model, "Identification.Queue"));
	processIdentification->setDelayExpression("tria(0.75, 1.5, 3)");
	processIdentification->setDelayTimeUnit(Util::TimeUnit::minute);

	Decide* decidePassSecurity = plugins->newInstance<Decide>(model);
	decidePassSecurity->setDescription("Pass security?");
	decidePassSecurity->getConditions()->insert("unif(0, 1) < 0.96");

	Dispose* disposeCleared = plugins->newInstance<Dispose>(model);
	disposeCleared->setDescription("Cleared");

	Dispose* disposeDenied = plugins->newInstance<Dispose>(model);
	disposeDenied->setDescription("Denied");

	Process* processXray = plugins->newInstance<Process>(model);
	processXray->setDescription("XRay Baggage Check");
	processXray->getSeizeRequests()->insert(new SeizableItem(xray, "1"));
	processXray->setQueueableItem(new QueueableItem(model, "XRay.Queue"));
	processXray->setDelayExpression("tria(1.75, 2.5, 7)");
	processXray->setDelayTimeUnit(Util::TimeUnit::minute);

	Assign* assignCounter = plugins->newInstance<Assign>(model);
	assignCounter->getAssignments()->insert(new Assignment(model, "v_Counter", "v_Counter + 1", false));

	Decide* decideExtraScreening = plugins->newInstance<Decide>(model);
	decideExtraScreening->setDescription("Extra screening for 15th?");
	decideExtraScreening->getConditions()->insert("v_Counter == 15");

	Assign* resetCounter = plugins->newInstance<Assign>(model);
	resetCounter->getAssignments()->insert(new Assignment(model, "v_Counter", "0", false));

	Process* extraSecurityCheck = plugins->newInstance<Process>(model);
	extraSecurityCheck->setDescription("Additional Security Check");
	extraSecurityCheck->setQueueableItem(new QueueableItem(model, "Additional.Queue"));
	extraSecurityCheck->setDelayExpression("tria(3, 5, 10)");
	extraSecurityCheck->setDelayTimeUnit(Util::TimeUnit::minute);

	Record* cycleTimeRecord = plugins->newInstance<Record>(model);
	cycleTimeRecord->setExpressionName("Cycle Time for Selected Passengers");
	cycleTimeRecord->setExpression("a_Time_in");

	Dispose* disposeClearedWithExtraCheck = plugins->newInstance<Dispose>(model);
	disposeClearedWithExtraCheck->setDescription("Cleared with extra check");

	create->getConnections()->insert(assignTimeIn);
	assignTimeIn->getConnections()->insert(processIdentification);
	processIdentification->getConnections()->insert(decidePassSecurity);
	decidePassSecurity->getConnections()->insert(processXray);
	processXray->getConnections()->insert(assignCounter);
	assignCounter->getConnections()->insert(decideExtraScreening);
	decideExtraScreening->getConnections()->insert(resetCounter);
	resetCounter->getConnections()->insert(extraSecurityCheck);
	extraSecurityCheck->getConnections()->insert(cycleTimeRecord);
	cycleTimeRecord->getConnections()->insert(disposeClearedWithExtraCheck);
	decideExtraScreening->getConnections()->insert(disposeCleared);
	decidePassSecurity->getConnections()->insert(disposeDenied);

	//genesys->getTracer()->setTraceLevel(TraceManager::Level::L9_mostDetailed);
	genesys->getTraceManager()->setTraceLevel(TraceManager::Level::L2_results);
	model->getSimulation()->setReplicationLength(12, Util::TimeUnit::hour);
	model->getSimulation()->setNumberOfReplications(3);
	model->getSimulation()->setWarmUpPeriod(0.5);
	model->getSimulation()->setWarmUpPeriodTimeUnit(Util::TimeUnit::hour);
	model->getSimulation()->start();

	delete genesys;
	return 0;
}

