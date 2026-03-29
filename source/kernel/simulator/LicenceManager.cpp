/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   LicenceManager.cpp
 * Author: rafael.luiz.cancian
 * 
 * Created on 29 de Maio de 2019, 11:45
 */

#include "LicenceManager.h"

//using namespace GenesysKernel;

LicenceManager::LicenceManager(Simulator* simulator) {
	_simulator = simulator;
	setDefaultLicenceAndLimits();
}

void LicenceManager::setDefaultLicenceAndLimits() {
	_licence = "LICENCE: Academic Mode.";
	_licence += " In academic mode this software has full functionality and executing training-size simulation models. ";
	_licence += " This software may be duplicated and used for educational purposes only;";
	_licence += " All changes and improvements must be submitted to the git repository using pull requests.";
	_licence += " Any commercial application is a violation of the license agreement.";
	_licence += " Designed and developed by prof. Dr. Ing Rafael Luiz Cancian, 2019-2023.";
	_licence += " Contacts: rafael.cancian@ufsc.br , cancian@lisha.ufsc.br";
	_activationCode = "";
	_components = 100;
	_datadefinitions = 300;
	_entities = 300;
	_hosts = 1;
	_threads = 1;
}

const std::string LicenceManager::showLicence() const {
	return _licence;
}

const std::string LicenceManager::showLimits() const {
	std::string msg = "LIMITS: Based on your licence and activation code, your simulator is running under the following limits" +
			std::string(": ") + std::to_string(_components) + " components" +
			", " + std::to_string(_datadefinitions) + " elements" +
			", " + std::to_string(_entities) + " entities" +
			", " + std::to_string(_hosts) + " hosts" +
			", " + std::to_string(_threads) + " threads.";
	return msg;
}

const std::string LicenceManager::showActivationCode() const {
	if (_activationCode == "")
		return "ACTIVATION CODE: Not found.";
	else
		return "ACTIVATION CODE: " + _activationCode;
}

bool LicenceManager::lookforActivationCode() {
	// @TODO: Not implemented yet
	return false;
}

bool LicenceManager::insertActivationCode() {
	// @TODO: Not implemented yet
	return false;
}

void LicenceManager::removeActivationCode() {
	this->setDefaultLicenceAndLimits();
}

unsigned int LicenceManager::getModelComponentsLimit() {
	return this->_components;
}

unsigned int LicenceManager::getModelDatasLimit() {
	return this->_datadefinitions;
}

unsigned int LicenceManager::getEntityLimit() {
	return this->_entities;
}

unsigned int LicenceManager::getHostsLimit() {
	return this->_hosts;
}

unsigned int LicenceManager::getThreadsLimit() {
	return this->_threads;
}
