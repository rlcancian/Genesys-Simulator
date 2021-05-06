/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   SimulationResponse.cpp
 * Author: rafael.luiz.cancian
 * 
 * Created on 10 de Outubro de 2018, 16:18
 */

#include "SimulationResponse.h"
//#include "AnalysisOfVariance_if.h"

//using namespace GenesysKernel;

SimulationResponse::SimulationResponse(std::string type, std::string name, GetterMember getterMember) {
	_type = type;
	_name = name;
	_getterMemberFunction = getterMember;
}

std::string SimulationResponse::show() {
	return "name=" + this->_name + ", type=" + this->_type;
}

std::string SimulationResponse::getName() const {
	return _name;
}

void SimulationResponse::setName(std::string name) {
	_name = name;
}

std::string SimulationResponse::getType() const {
	return _type;
}

double SimulationResponse::getValue() {
	return this->_getterMemberFunction();
}