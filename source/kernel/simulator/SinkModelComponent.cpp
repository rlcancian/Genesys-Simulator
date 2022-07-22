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

bool SinkModelComponent::_loadInstance(PersistenceRecord *fields) {
	bool res = ModelComponent::_loadInstance(fields);
	return res;
}

//void SinkModelComponent::_initBetweenReplications() {}

void SinkModelComponent::_saveInstance(PersistenceRecord *fields, bool saveDefaultValues) {
	ModelComponent::_saveInstance(fields, saveDefaultValues);
}

bool SinkModelComponent::_check(std::string* errorMessage) {
	// @TODO: To implement!
	*errorMessage += "";
	return true;
}