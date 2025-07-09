/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */


#include "Rectifier.h"

// you have to included need libs

// GEnSyS Simulator
#include "../../../../kernel/simulator/Simulator.h"

// Model Components
#include "../../../../plugins/components/SPICECircuit.h"
#include "../../../../plugins/components/SPICENode.h"
#include "../../../../plugins/data/SPICERunner.h"
#include "../../../TraitsApp.h"

Rectifier::Rectifier() {
}

/**
 * This is the main function of the application. 
 * It instanciates the simulator, builds a simulation model and then simulate that model.
 */
int Rectifier::main(int argc, char** argv) {

	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();
	// create model
	SPICERunner* runner = plugins->newInstance<SPICERunner>(model);

	SPICENode* gnd = plugins->newInstance<SPICENode>(model, "gnd");
    SPICENode* vdd = plugins->newInstance<SPICENode>(model);
	SPICENode* vm = plugins->newInstance<SPICENode>(model);
	SPICENode* vp = plugins->newInstance<SPICENode>(model);

    double sim_time = 1, sim_step = 0.001;
    double voltage = 0.9;

	Vsine* vpu1 = plugins->newInstance<Vsine>(model);
	vpu1->setRunner(runner);
	vpu1->setVoltage(12);
	vpu1->setFreq(60);
	vpu1->getConnectionManager()->insertAtPort(0, new Connection({vdd, 0}));
	vpu1->getConnectionManager()->insertAtPort(1, new Connection({gnd, 0}));

	Diode* d1 = plugins->newInstance<Diode>(model);
	d1->setRunner(runner);
	d1->getConnectionManager()->insertAtPort(0, new Connection({vm, 0}));
	d1->getConnectionManager()->insertAtPort(1, new Connection({vdd, 0}));

	Diode* d2 = plugins->newInstance<Diode>(model);
	d2->setRunner(runner);
	d2->getConnectionManager()->insertAtPort(0, new Connection({vm, 0}));
	d2->getConnectionManager()->insertAtPort(1, new Connection({gnd, 0}));

	Diode* d3 = plugins->newInstance<Diode>(model);
	d3->setRunner(runner);
	d3->getConnectionManager()->insertAtPort(0, new Connection({vdd, 0}));
	d3->getConnectionManager()->insertAtPort(1, new Connection({vp, 0}));

	Diode* d4 = plugins->newInstance<Diode>(model);
	d4->setRunner(runner);
	d4->getConnectionManager()->insertAtPort(0, new Connection({gnd, 0}));
	d4->getConnectionManager()->insertAtPort(1, new Connection({vm, 0}));

	Capacitor* c1 = plugins->newInstance<Capacitor>(model);
	c1->setRunner(runner);
	c1->setCapacitance(0.01);
	c1->getConnectionManager()->insertAtPort(0, new Connection({vp, 0}));
	c1->getConnectionManager()->insertAtPort(1, new Connection({vm, 0}));

	Resistor* r1 = plugins->newInstance<Resistor>(model);
	r1->setRunner(runner);
	r1->setResistance(1000);
	r1->getConnectionManager()->insertAtPort(0, new Connection({vp, 0}));
	r1->getConnectionManager()->insertAtPort(1, new Connection({vm, 0}));

	runner->PlotVRelative(vm->getNodeName(), vp->getNodeName());
	runner->ConfigSim(sim_time, sim_step);
	runner->Run();

	model->save("rectifier.gen");

	delete genesys;

	return 0;
};

