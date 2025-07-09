/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */


#include "Half_Adder.h"

// you have to included need libs

// GEnSyS Simulator
#include "../../../../kernel/simulator/Simulator.h"

// Model Components
#include "../../../../plugins/components/SPICECircuit.h"
#include "../../../../plugins/components/SPICENode.h"
#include "../../../../plugins/data/SPICERunner.h"
#include "../../../TraitsApp.h"

Half_Adder::Half_Adder() {
}

/**
 * This is the main function of the application. 
 * It instanciates the simulator, builds a simulation model and then simulate that model.
 */
int Half_Adder::main(int argc, char** argv) {

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
    SPICENode* a = plugins->newInstance<SPICENode>(model);
    SPICENode* b = plugins->newInstance<SPICENode>(model);
    SPICENode* sum = plugins->newInstance<SPICENode>(model);
    SPICENode* carry = plugins->newInstance<SPICENode>(model);


    double sim_time = 1, sim_step = 0.001;
    double voltage = 0.9;

	Vsource* v1 = plugins->newInstance<Vsource>(model);
	v1->setRunner(runner);
	v1->setVoltage(voltage);
	v1->getConnectionManager()->insertAtPort(0, new Connection({vdd, 0}));
	v1->getConnectionManager()->insertAtPort(1, new Connection({gnd, 0}));

	Vpulse* va = plugins->newInstance<Vpulse>(model);
	va->setRunner(runner);
	va->setVoltage(voltage);
	va->setFreq(sim_time);
	va->getConnectionManager()->insertAtPort(0, new Connection({a, 0}));
	va->getConnectionManager()->insertAtPort(1, new Connection({gnd, 0}));

	Vpulse* vb = plugins->newInstance<Vpulse>(model);
	vb->setRunner(runner);
	vb->setVoltage(voltage);
	vb->setFreq(sim_time/2);
	vb->getConnectionManager()->insertAtPort(0, new Connection({b, 0}));
	vb->getConnectionManager()->insertAtPort(1, new Connection({gnd, 0}));

	XNOR* xnor = plugins->newInstance<XNOR>(model);
	xnor->setRunner(runner);
	xnor->getConnectionManager()->insertAtPort(0, new Connection({vdd, 0}));
	xnor->getConnectionManager()->insertAtPort(1, new Connection({gnd, 0}));
	xnor->getConnectionManager()->insertAtPort(2, new Connection({a, 0}));
	xnor->getConnectionManager()->insertAtPort(3, new Connection({b, 0}));
	xnor->getConnectionManager()->insertAtPort(4, new Connection({sum, 0}));

	NAND* nand = plugins->newInstance<NAND>(model);
	nand->setRunner(runner);
	nand->getConnectionManager()->insertAtPort(0, new Connection({vdd, 0}));
	nand->getConnectionManager()->insertAtPort(1, new Connection({gnd, 0}));
	nand->getConnectionManager()->insertAtPort(2, new Connection({a,0}));
	nand->getConnectionManager()->insertAtPort(3, new Connection({b, 0}));
	nand->getConnectionManager()->insertAtPort(4, new Connection({carry, 0}));

	// runner->ConfigSim(sim_time, sim_step, "v("+sum->getNodeName()+")");
	// runner->Run(runner->CompileSpiceFile());

	delete genesys;

	return 0;
};

