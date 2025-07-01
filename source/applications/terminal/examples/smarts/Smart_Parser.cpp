/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Smart_Parser.cpp
 * Author: rlcancian
 * 
 * Created on 5 de Janeiro de 2022, 07:21
 */

#include "Smart_Parser.h"

// you have to included need libs

// GEnSyS Simulator
#include "../../../../kernel/simulator/Simulator.h"

// Model Components
#include "../../../../plugins/components/Create.h"
#include "../../../../plugins/components/Dispose.h"
#include "../../../TraitsApp.h"

Smart_Parser::Smart_Parser() {
}

/**
 * This is the main function of the application. 
 * It instanciates the simulator, builds a simulation model and then simulate that model.
 */
int Smart_Parser::main(int argc, char** argv) {
	Simulator* genesys = new Simulator();
	genesys->getTraceManager()->setTraceLevel(TraitsApp<GenesysApplication_if>::traceLevel);
	setDefaultTraceHandlers(genesys->getTraceManager());
	PluginManager* plugins = genesys->getPluginManager();
	plugins->autoInsertPlugins("autoloadplugins.txt");
	Model* model = genesys->getModelManager()->newModel();
	// Just parser evaluating different expressions
	// (arithmetic, trigonometric, matemathic, logic, statistic and conditional)
	std::vector<std::string> expressions = {
		"1e-3",
		"0x2fE-0x1b",
		"(1.0<=2.5)==0",
		"min(10e1, 1e-3)", //@TODO!!! THE RESULT IS WRONG
		"-3 + 2*0.25 +0xc/4 -1.34e-3 *(-1*-1)-2^5",
		"sin(0.3) - cos(1.75)",
		"trunc(12.99) + round(12.99) + frac(45.009)",
		"exp(1) -ln(exp(1)) + log(10) + sqrt(81) - mod(15,8)",
		"not false and (3 nand (0 xor false))",
		"norm(0,1) + expo(10) + unif(2,5) + weib(2,1) + logn(10,1) + erla(2,1) + beta(6,2,1,1) + tria(1,4,5)",
		"disc(0.1,1,0.3,2,0.6,3,1,4)", // not implemented in parser
		"if false and true 0 else 1",
		"if false or true 0 else 1",
		"if 1 if 1 2 else 3",
		"if 1 if 0 2 else 3",
		"if 1 (if 0 2) else 3",
		"if false if true 0 else 1 else if true 2 else 3"
	};
	// an empty model is created since parser is attached to a model
	for (unsigned int i = 0; i < expressions.size(); i++) {
		std::cout << "\"" << expressions[i] << "\" = " << model->parseExpression(expressions[i]) << std::endl;
	}
	delete genesys;
	return 0;
};

