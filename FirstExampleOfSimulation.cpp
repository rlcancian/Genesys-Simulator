/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   FirstExampleOfSimulation.cpp
 * Author: rlcancian
 * 
 * Created on 3 de Setembro de 2019, 18:34
 */
#include <fstream>
#include <iostream>

#include "FirstExampleOfSimulation.h"

#include "SamplerDefaultImpl1.h"

FirstExampleOfSimulation::FirstExampleOfSimulation() {
}

/**
 * This is the main function of the application. 
 * It instanciates the simulator, builds a simulation model and then simulate that model.
 */
int FirstExampleOfSimulation::main(int argc, char** argv) {
	SamplerDefaultImpl1 sampler = SamplerDefaultImpl1();

	std::ofstream file;
	file.open("Chi2(2)-Output");

	for (int i = 0; i < 1000; i++) {
		file << sampler.sampleChiSqrt(2) << std::endl;
	}
	file.close();

	sampler.reset();
	file.open("Gumbell(1,2)-Output");
	for (int i = 0; i < 1000; i++) {
		file << sampler.sampleGumbell(1,2) << std::endl;
	}
	file.close();

	sampler.reset();
	file.open("GumbellInv(1,2)-Output");
	for (int i = 0; i < 1000; i++) {
		file << sampler.sampleGumbellInv(1,2) << std::endl;
	}
	file.close();

	sampler.reset();
	file.open("Beta(1,2)-Output");
	for (int i = 0; i < 1000; i++) {
		file << sampler.sampleBetaPDF(1,2) << std::endl;
	}
	file.close();

	sampler.reset();
	file.open("Gamma(1,2)-Output");
	for (int i = 0; i < 1000; i++) {
		file << sampler.sampleGammaPDF(1,2) << std::endl;
	}
	file.close();

	sampler.reset();
	file.open("sampleBinomial(10)-Output");
		for(int i = 0; i < 1000; i++){
		file << sampler.sampleBinomial(10) << std::endl;
	}
	file.close();

	sampler.reset();
	file.open("sampleGeometric(0.02)-Output");
		for(int i = 0; i < 100; i++){
		file << sampler.sampleGeometric(i, 0.02) << std::endl;
	}
	file.close();

	return 0;
};

