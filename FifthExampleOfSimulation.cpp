/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   FifthExampleOfSimulation.cpp
 * Author: rlcancian
 * 
 * Created on 12 de março de 2021, 15:39
 */

#include "FifthExampleOfSimulation.h"
#include "BaseConsoleGenesysApplication.h"
#include "Simulator.h"
#include "Create.h"
#include "Route.h"
#include "Enter.h"
#include "Leave.h"
#include "Dispose.h"
#include "Station.h"
#include "EntityType.h"
#include "Delay.h"
#include "Sequence.h"
#include "Assign.h"

FifthExampleOfSimulation::FifthExampleOfSimulation() {
}

int FifthExampleOfSimulation::main(int argc, char** argv) {
	Simulator* sim = new Simulator();
	this->insertFakePluginsByHand(sim);
	this->setDefaultTraceHandlers(sim->getTracer());
	sim->getTracer()->setTraceLevel(Util::TraceLevel::componentArrival);
	Model* m = sim->getModels()->newModel();
	Create* c1 = new Create(m);
	c1->setEntityType(new EntityType(m));
	c1->setTimeBetweenCreationsExpression("10");
	Station* s1 = new Station(m);
	Route* r0 = new Route(m);
	r0->setRouteDestinationType(Route::DestinationType::BySequence); //(Route::DestinationType::Station);
	//r0->setStation(s1);
	r0->setRouteTimeExpression("0.1");
	Enter* e1 = new Enter(m);
	e1->setStation(s1);
	Delay* d1 = new Delay(m);
	Station* s2 = new Station(m);
	Leave* l1 = new Leave(m);
	l1->setStation(s1);
	Route* r1 = new Route(m);
	r1->setRouteDestinationType(Route::DestinationType::BySequence); //(Route::DestinationType::Station);
	//r1->setStation(s2);
	r1->setRouteTimeExpression("0.2");
	Enter* e2 = new Enter(m);
	e2->setStation(s2);
	Delay* d2 = new Delay(m);
	d2->setDelayExpression("2");
	Leave* l2 = new Leave(m);
	l2->setStation(s2);
	Station* s3 = new Station(m);
	Route* r2 = new Route(m);
	r2->setRouteDestinationType(Route::DestinationType::BySequence); //(Route::DestinationType::Station);
	//r2->setStation(s3);
	r2->setRouteTimeExpression("0.3");
	Enter* e3 = new Enter(m);
	e3->setStation(s3);
	Dispose* dp1 = new Dispose(m);

	Sequence* seq = new Sequence(m);
	seq->getSteps()->insert(new SequenceStep(s1));
	seq->getSteps()->insert(new SequenceStep(s2));
	seq->getSteps()->insert(new SequenceStep(s1));
	seq->getSteps()->insert(new SequenceStep(s1));
	seq->getSteps()->insert(new SequenceStep(s3));

	Assign* a1 = new Assign(m);
	a1->getAssignments()->insert(new Assign::Assignment("Entity.Sequence", std::to_string(seq->getId())));

	c1->getNextComponents()->insert(a1);
	a1->getNextComponents()->insert(r0);
	e1->getNextComponents()->insert(d1);
	d1->getNextComponents()->insert(l1);
	l1->getNextComponents()->insert(r1);
	e2->getNextComponents()->insert(d2);
	d2->getNextComponents()->insert(l2);
	l2->getNextComponents()->insert(r2);
	e3->getNextComponents()->insert(dp1);
	m->getInfos()->setReplicationLength(60);
	m->getSimulation()->start();
	return 0;
}