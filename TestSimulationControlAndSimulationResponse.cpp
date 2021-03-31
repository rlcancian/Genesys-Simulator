/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   TestSimulationControlAndSimulationResponse.cpp
 * Author: rlcancian
 * 
 * Created on 11 de Setembro de 2019, 17:44
 */

#include "TestSimulationControlAndSimulationResponse.h"
#include "Simulator.h"
#include "Model.h"
#include "SimulationControl.h"
#include "SimulationResponse.h"
#include <iostream>

TestSimulationControlAndSimulationResponse::TestSimulationControlAndSimulationResponse() {
}

int TestSimulationControlAndSimulationResponse::main(int argc, char** argv) {
	Simulator* simulator = new Simulator();
	TraceManager* tm = simulator->getTracer();
	this->setDefaultTraceHandlers(tm);
	tm->setTraceLevel(Util::TraceLevel::componentDetailed);
	this->insertFakePluginsByHand(simulator);

	simulator->getModels()->loadModel("./temp/forthExampleOfSimulation.txt");
	Model* model = simulator->getModels()->current();
	model->check();

	tm->trace("Controls:");
	for (std::list<SimulationControl*>::iterator it = model->getControls()->list()->begin(); it != model->getControls()->list()->end(); it++) {
		tm->trace((*it)->getName() + ": " + std::to_string((*it)->getValue()));
	}


	tm->trace("\nResponses:");
	for (std::list<SimulationResponse*>::iterator it = model->getResponses()->list()->begin(); it != model->getResponses()->list()->end(); it++) {
		tm->trace((*it)->getName() + ": " + std::to_string((*it)->getValue()));
	}


	/*
	Model* model = new Model(simulator);
	model->show();

	std::cout << "NumRepl antes: " << model->getInfos()->getNumberOfReplications() << std::endl;
	model->getInfos()->setNumberOfReplications(10);
	std::cout << "NumRepl depois: " << model->getInfos()->getNumberOfReplications() << std::endl;
	SimulationControl* control = model->getControls()->front();
	std::cout << control->getName() << " antes: " << control->getValue() << std::endl;
	control->setValue(20);
	std::cout << control->getName() << " depois: " << control->getValue() << std::endl;
	std::cout << "NumRepl depois: " << model->getInfos()->getNumberOfReplications() << std::endl;
	 */
	return 0;
}