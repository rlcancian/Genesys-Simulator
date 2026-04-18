/*
 * File:   Smart_R_Simulator.cpp
 * Author: GenESyS
 *
 * Minimal smart model for the RSimulator component.
 */

#include "Smart_R_Simulator.h"

#include "kernel/simulator/Simulator.h"
#include "plugins/components/DiscreteProcessing/Create.h"
#include "plugins/components/DiscreteProcessing/Dispose.h"
#include "plugins/components/ExternalIntegration/RSimulator.h"
#include "../../../TraitsApp.h"

Smart_R_Simulator::Smart_R_Simulator() {
}

int Smart_R_Simulator::main(int argc, char** argv) {
	(void) argc;
	(void) argv;

	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());

	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();

	Create* create = plugins->newInstance<Create>(model);
	create->setEntityTypeName("RJob");
	create->setTimeBetweenCreationsExpression("1", Util::TimeUnit::second);
	create->setEntitiesPerCreation(1);
	create->setMaxCreations("3");

	RSimulator* rSimulator = plugins->newInstance<RSimulator>(model);
	if (rSimulator == nullptr) {
		genesys->getTraceManager()->traceError("RSimulator plugin was not inserted; creating the component directly for this smoke model. If Rscript is unavailable, the component will trace execution failures at runtime.");
		rSimulator = new RSimulator(model, "RSimulator_1");
	}
	rSimulator->setPreludeScript("genesys_label <- 'RSimulator'");
	rSimulator->insertCommand("cat(genesys_label, ': entity command 1 executed\\n')");
	rSimulator->insertCommand("cat('R mean example =', mean(c(10, 20, 30)), '\\n')");

	Dispose* dispose = plugins->newInstance<Dispose>(model);

	create->getConnectionManager()->insert(rSimulator);
	rSimulator->getConnectionManager()->insert(dispose);

	model->getSimulation()->setReplicationLength(5);
	model->getSimulation()->setTerminatingCondition("");
	model->getSimulation()->setShowReportsAfterReplication(false);
	model->getSimulation()->setShowReportsAfterSimulation(true);
	model->save("./models/Smart_R_Simulator.gen");
	model->getSimulation()->start();

	delete genesys;
	return 0;
}
