/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Smart_Attribute_Variable.cpp
 * Author: rlcancian
 * 
 * Created on 3 de Setembro de 2019, 18:34
 */

#include "Smart_Attribute_Variable.h"

// you have to included need libs

// GEnSyS Simulator
#include "kernel/simulator/Simulator.h"

// Model Components
#include "plugins/components/Logic/Create.h"
#include "plugins/components/Logic/Assign.h"
#include "plugins/components/Logic/Dispose.h"
#include "../../../TraitsApp.h"

Smart_Attribute_Variable::Smart_Attribute_Variable() {
}

/**
 * This is the main function of the application. 
 * It instanciates the simulator, builds a simulation model and then simulate that model.
 */
int Smart_Attribute_Variable::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins();
	Model* model = genesys->getModelManager()->newModel();
	// create model
	Create* create1 = plugins->newInstance<Create>(model);
	Assign* assign1 = plugins->newInstance<Assign>(model);
	// Assign evaluates expressions as scalars today.
	// Vectors and matrices are built by assigning each element explicitly.
	// The index syntax used by the runtime is 0-based.
	assign1->addAssignment(new Assignment("atrScalar", "3.14", true));

	// 1D vector attribute.
	Attribute* atr2 =  plugins->newInstance<Attribute>(model, "atr2");
	atr2->setInitialValuesText("[0.1 0.2 0.3]");
	assign1->addAssignment(new Assignment("atr2[0]", "1.1 + atr2[0]", true));
	assign1->addAssignment(new Assignment("atr2[1]", "1.2 + atr2[0]", true));
	assign1->addAssignment(new Assignment("atr2[2]", "1.3 + atr2[1]", true));
	assign1->addAssignment(new Assignment("atrVecCopy", "atr2[1]", true));

	// 2D matrix attribute.
	assign1->addAssignment(new Assignment("atrMat[0,0]", "1.1", true));
	assign1->addAssignment(new Assignment("atrMat[0,1]", "1.2", true));
	assign1->addAssignment(new Assignment("atrMat[0,2]", "1.3", true));
	assign1->addAssignment(new Assignment("atrMat[1,0]", "2.1", true));
	assign1->addAssignment(new Assignment("atrMat[1,1]", "2.2", true));
	assign1->addAssignment(new Assignment("atrMat[1,2]", "2.3", true));
	assign1->addAssignment(new Assignment("atrMat[2,0]", "3.1", true));
	assign1->addAssignment(new Assignment("atrMat[2,1]", "3.2", true));
	assign1->addAssignment(new Assignment("atrMat[2,2]", "3.3", true));
	assign1->addAssignment(new Assignment("atrMatCopy[0,0]", "atrMat[2,0]", true));

	// Variable equivalents use the same 0-based index syntax.
	assign1->addAssignment(new Assignment("varScalar", "7.5 + varScalar", false));
	assign1->addAssignment(new Assignment("var2[0]", "var2[0] + 1", false));
	assign1->addAssignment(new Assignment("var2[1]", "var2[1]", false));
	assign1->addAssignment(new Assignment("var2[2]", "var2[2] + var2[0]", false));
	assign1->addAssignment(new Assignment("varVecCopy", "var2[1]", false));
	assign1->addAssignment(new Assignment("varMat[0,0]", "21", false));
	assign1->addAssignment(new Assignment("varMat[0,1]", "2 + varMat[0,1]", false));
	assign1->addAssignment(new Assignment("varMat[1,0]", "1 + varMat[1,1]", false));
	assign1->addAssignment(new Assignment("varMat[1,1]", "2 + varMat[1,1]", false));
	assign1->addAssignment(new Assignment("varMatCopy[0,0]", "varMat[1,1]", false));
	Dispose* dispose1 = plugins->newInstance<Dispose>(model);
	// connect model components to create a "workflow"
	create1->connectTo(assign1);
	assign1->connectTo(dispose1);
	// set options, save and simulate
	model->getSimulation()->setReplicationLength(60, Util::TimeUnit::second);
	model->save("./models/Smart_Attribute_Variable.gen");
	model->getSimulation()->start();
	delete genesys;
	return 0;
};
