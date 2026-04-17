#include "TestingTerminalApp.h"

// you have to included need libs

// GEnSyS Simulator
#include "kernel/simulator/Simulator.h"

// Model Components
#include "plugins/components/DiscreteProcessing/Create.h"
#include "plugins/components/DiscreteProcessing/Delay.h"
#include "plugins/components/DiscreteProcessing/Dispose.h"

TestingTerminalApp::TestingTerminalApp()
{

}

/**
 * This is the main function of the application.
 * It instanciates the simulator, builds a simulation model and then simulate that model.
 */
void TestingTerminalApp::Simulate(Simulator* genesys, Model* model, PluginManager *plugins) { //main(int argc, char** argv) {
	Create* create1 = plugins->newInstance<Create>(model);
	Delay* delay1 = plugins->newInstance<Delay>(model);
	Dispose* dispose1 = plugins->newInstance<Dispose>(model);
	create1->getConnections()->insert(delay1);
	delay1->getConnections()->insert(dispose1);
	model->getSimulation()->start();
	delete genesys;
};
