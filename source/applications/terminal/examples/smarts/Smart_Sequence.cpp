/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Smart_Sequence.cpp
 * Author: rlcancian
 * 
 * Created on 12 de março de 2021, 15:39
 */

#include "Smart_Sequence.h"

#include "../../../../kernel/simulator/Simulator.h"

#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/Route.h"
#include "../../../../plugins/components/Enter.h"
#include "../../../../plugins/components/Leave.h"
#include "../../../../plugins/components/Dispose.h"
#include "../../../../plugins/components/Delay.h"
#include "../../../../plugins/components/Assign.h"

#include "../../../../plugins/data/Sequence.h"
#include "../../../../plugins/data/Station.h"
#include "../../../TraitsApp.h"

Smart_Sequence::Smart_Sequence() {
}

int Smart_Sequence::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();
	// create model
	model->getSimulation()->setReplicationLength(60);
	Create* c1 = plugins->newInstance<Create>(model);
	c1->setTimeBetweenCreationsExpression("10");
	Route* r0 = plugins->newInstance<Route>(model);
	r0->setRouteDestinationType(Route::DestinationType::Sequence);
	r0->setRouteTimeExpression("0.1");
	Enter* e1 = plugins->newInstance<Enter>(model);
	e1->setStationName("s1");
	Delay* d1 = plugins->newInstance<Delay>(model);
	Leave* l1 = plugins->newInstance<Leave>(model);
	l1->setStationName("s1");
	Route* r1 = plugins->newInstance<Route>(model);
	r1->setRouteDestinationType(Route::DestinationType::Sequence);
	r1->setRouteTimeExpression("0.2");
	Enter* e2 = plugins->newInstance<Enter>(model);
	e2->setStationName("s2");
	Delay* d2 = plugins->newInstance<Delay>(model);
	d2->setDelayExpression("2");
	Leave* l2 = plugins->newInstance<Leave>(model);
	l2->setStationName("s2");
	Route* r2 = plugins->newInstance<Route>(model);
	r2->setRouteDestinationType(Route::DestinationType::Sequence);
	r2->setRouteTimeExpression("0.3");
	Enter* e3 = plugins->newInstance<Enter>(model);
	e3->setStationName("s3");
	Dispose* dp1 = plugins->newInstance<Dispose>(model);
	// now defines the sequence of stations
	Sequence* seq = new Sequence(model, "mysequence");
	seq->getSteps()->insert(new SequenceStep(model, "s1"));
	seq->getSteps()->insert(new SequenceStep(model, "s2"));
	seq->getSteps()->insert(new SequenceStep(model, "s1"));
	seq->getSteps()->insert(new SequenceStep(model, "s1"));
	seq->getSteps()->insert(new SequenceStep(model, "s3"));
	// finally defines the created entities will follow this sequence
	Assign* a1 = plugins->newInstance<Assign>(model);
	a1->getAssignments()->insert(new Assignment("Entity.Sequence", std::to_string(seq->getId()))); //@TODO: Implement in parser to be possible to extract ID from SequenceName
	// connect model components to create a "workflow"
	c1->getConnections()->insert(a1);
	a1->getConnections()->insert(r0);
	e1->getConnections()->insert(d1);
	d1->getConnections()->insert(l1);
	l1->getConnections()->insert(r1);
	e2->getConnections()->insert(d2);
	d2->getConnections()->insert(l2);
	l2->getConnections()->insert(r2);
	e3->getConnections()->insert(dp1);
	// set options, save and simulate (some breakpoins are created)
	ModelSimulation* sim = model->getSimulation();
	sim->getBreakpointsOnComponent()->insert(a1);
	sim->getBreakpointsOnComponent()->insert(l2);
	sim->getBreakpointsOnTime()->insert(40.0);
	sim->getBreakpointsOnTime()->insert(20.0);
	sim->setShowReportsAfterReplication(false);
	sim->setShowReportsAfterSimulation(false);
	model->save("./models/Smart_Sequence.gen");
	do {
		sim->start();
	} while (sim->isPaused());
	delete genesys;
	return 0;
}
