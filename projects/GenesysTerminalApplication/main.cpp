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

#include "../../source/applications/TraitsApp.h"
/*!
 * This is the MAIN application of GenESyS. It just calls the Application specificied on the configuration/traits file.
 * This file should NOT be changed. Configure the application to be executed using TraitsApp<GenesysApplication_if>
 */
int main(int argc, char** argv) {  
	// do not change it. Set you own application in Traits file => TraitsApp<GenesysApplication_if>::Application
	GenesysApplication_if *app = new TraitsApp<GenesysApplication_if>::Application();
	int res = app->main(argc, argv);
	return res;
}
