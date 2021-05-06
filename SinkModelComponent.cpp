/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   SinkModelComponent.cpp
 * Author: rafael.luiz.cancian
 * 
 * Created on 14 de Agosto de 2018, 14:29
 */

#include "SinkModelComponent.h"

//using namespace GenesysKernel;

SinkModelComponent::SinkModelComponent(Model* model, std::string componentTypename, std::string name) : ModelComponent(model, componentTypename, name) {
}

bool SinkModelComponent::_loadInstance(std::map<std::string, std::string>* fields) {
	bool res = ModelComponent::_loadInstance(fields);
	if (res) {
		//this->_reportStatistics = std::stoi((*fields->find("collectStatistics")).second) != 0;
	}
	return res;
}

void SinkModelComponent::_initBetweenReplications() {
}

std::map<std::string, std::string>* SinkModelComponent::_saveInstance() {
	std::map<std::string, std::string>* fields = ModelComponent::_saveInstance();
	return fields;
}

bool SinkModelComponent::_check(std::string* errorMessage) {
	return true;
}