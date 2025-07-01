/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/class.cc to edit this template
 */

/* 
 * File:   Smart_Plugin.cpp
 * Author: rlcancian
 * 
 * Created on 13 de janeiro de 2022, 17:47
 */

#include "Smart_Plugin.h"

#include "../../../../kernel/simulator/Simulator.h"
#include "../../../TraitsApp.h"

Smart_Plugin::Smart_Plugin() {
}

int Smart_Plugin::main(int argc, char** argv) {
	Simulator* simulator = new Simulator();
	simulator->getPluginManager()->autoInsertPlugins("autoloadplugins.txt");
	// just show information about the connected plugins
	for (unsigned int i = 0; i < simulator->getPluginManager()->size(); i++) {
		std::cout << std::endl << simulator->getPluginManager()->getAtRank(i)->show() << std::endl;
	}
	delete simulator;
	return 0;
}
