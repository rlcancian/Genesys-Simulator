/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/class.cc to edit this template
 */

/* 
 * File:   Smart_CppForG.cpp
 * Author: rlcancian
 * 
 * Created on 11 de janeiro de 2022, 22:39
 */

#include "Smart_CppForG.h"

#include "../../../../kernel/simulator/Simulator.h"
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/CppForG.h"
#include "../../../../plugins/components/Dispose.h"
#include "../../../../kernel/simulator/ModelSimulation.h"
#include "../../../TraitsApp.h"

Smart_CppForG::Smart_CppForG() {
}

int Smart_CppForG::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();
	// create model
	Create* create = plugins->newInstance<Create>(model);
	CppForG* cpp1 = plugins->newInstance<CppForG>(model);
	CppForG* cpp2 = plugins->newInstance<CppForG>(model);
	Dispose* dispose = plugins->newInstance<Dispose>(model);
	// connect model components to create a "workflow"
	create->getConnections()->insert(cpp1);
	cpp1->getConnections()->insert(cpp2);
	cpp2->getConnections()->insert(dispose);
	// set parameters for cpp1 and cpp2 components
	std::string includeCode = "\
		#include \"../../../../kernel/simulator/ModelSimulation.h\"\n\
		#include \"../../../../kernel/simulator/ModelDataDefinition.h\"\n\
";
	cpp1->setIncludesCode(includeCode);
	cpp2->setIncludesCode(includeCode);
	std::string cppDispachEventCode = "\
	std::cout << \"Executando código do usuário\" << std::endl;\n\
	std::cout << \"Os ponteiros são \" << simulator << \" , \" << model << \" , \" << entity << std::endl;\n\
	std::cout << \"O nome desta entidade é \" << entity->getName() << std::endl;\n\
	unsigned int numComponentes = model->getComponents()->getAllComponents()->size();\n\
	std::cout << \"Este modelo tem \" << numComponentes << \" componentes\" << std::endl;\n\
	double tnow = model->getSimulation()->getSimulatedTime();\n\
	std::cout << \"O tempo simulado atual é \" << tnow << std::endl;\n\
	";
	cpp1->setOnDispatchEventCode(cppDispachEventCode);
	cpp2->setOnDispatchEventCode(cppDispachEventCode);
	std::string initCode = "\
	std::cout << \"Inicializando meu código 1\" << std::endl;\n\
	// mais nada\
	";
	cpp1->setInitBetweenReplicationCode(initCode);
	initCode = "std::cout << \"Inicializando meu código 2\" << std::endl;\n";
	cpp2->setInitBetweenReplicationCode(initCode);
	// set options, save and simulate
	ModelSimulation* s = model->getSimulation();
	s->setReplicationLength(10);
	s->setShowReportsAfterReplication(false);
	s->setShowReportsAfterSimulation(false);
	model->save("./models/Smart_CppForG.gen");
	s->start();
	delete genesys;
	return 0;
}

