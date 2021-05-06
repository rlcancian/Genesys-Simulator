/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cpp
 * Author: rafael.luiz.cancian
 *
 * Created on 21 de Junho de 2018, 12:47
 */

#include <iostream>
#include <thread>
#include <limits>
#include "Traits.h"

/*
 * This is the MAIN application of GenESyS. It just calls the Application specificied on the configuration file.
 */
int main(int argc, char** argv) {
	// do not change it. Set you own application in Traits file => Traits<GenesysApplication_if>::Application
	GenesysApplication_if *app = new Traits<GenesysApplication_if>::Application();
	int res = app->main(argc, argv);
	// that's all folks!!
	std::cout << "Press ENTER to quit...";
	std::cin.ignore(std::numeric_limits <std::streamsize> ::max(), '\n');
	return res;
}