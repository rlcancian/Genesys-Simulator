/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/class.cc to edit this template
 */

/* 
 * File:   Book_Cap02_Example01.cpp
 * Author: rlcancian
 * 
 * Created on 19 de outubro de 2021, 16:23
 */

#include "Book_Cap02_Example01.h"
#include "../../../../kernel/simulator/Simulator.h"
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/Delay.h"
#include "../../../../plugins/components/Dispose.h"

Book_Cap02_Example01::Book_Cap02_Example01() {
}

int Book_Cap02_Example01::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	this->setDefaultTraceHandlers(genesys->getTraceManager());
	genesys->getPluginManager()->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();
	PluginManager* plugins = genesys->getPluginManager();
	Create* create1 = plugins->newInstance<Create>(model);
	//EntityType* entityType1 = plugins->newInstance<EntityType>(model, "EntityType1");
	//create1->setEntityType(entityType1);
	create1->setTimeBetweenCreationsExpression("NORM(5,2)");
	Delay* delay1 = plugins->newInstance<Delay>(model);
	delay1->setDelayExpression("unif(3,7)");
	Dispose* dispose1 = plugins->newInstance<Dispose>(model);
	create1->getConnectionManager()->insert(delay1);
	delay1->getConnectionManager()->insert(dispose1);
	model->getSimulation()->setReplicationLength(30, Util::TimeUnit::second);
	model->getSimulation()->setNumberOfReplications(100);
	model->getSimulation()->start();
	delete genesys;
	return 0;
}

