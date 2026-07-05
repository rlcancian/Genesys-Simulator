#include "Smart_PythonForG.h"

#include "kernel/simulator/Simulator.h"
#include "kernel/simulator/model/ModelSimulation.h"
#include "plugins/components/ExternalIntegration/PythonForG.h"
#include "plugins/components/Logic/Create.h"
#include "plugins/components/Logic/Dispose.h"
#include "../../../TraitsApp.h"

Smart_PythonForG::Smart_PythonForG() = default;

int Smart_PythonForG::main(int argc, char** argv) {
	(void) argc;
	(void) argv;

	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");

	Model* model = genesys->getModelManager()->newModel();
	Create* create = plugins->newInstance<Create>(model);
	PythonForG* python = plugins->newInstance<PythonForG>(model);
	Dispose* dispose = plugins->newInstance<Dispose>(model);

	create->getConnectionManager()->insert(python);
	python->getConnectionManager()->insert(dispose);

	python->setInitBetweenReplicationCode(
			"context.log('PythonForG init hook running')\n"
			"simulator.infoSetDescription('initialized from PythonForG')\n");
	python->setOnDispatchEventCode(
			"context.log('dispatching entity ' + entity.getName())\n"
			"context.log('sim time=' + str(simulator.simGetSimulatedTime()))\n");

	ModelSimulation* simulation = model->getSimulation();
	simulation->setReplicationLength(5.0);
	simulation->setShowReportsAfterReplication(false);
	simulation->setShowReportsAfterSimulation(false);
	model->save("./models/Smart_PythonForG.gen");
	simulation->start();

	delete genesys;
	return 0;
}
