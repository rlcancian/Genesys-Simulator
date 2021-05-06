/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ModelManager.cpp
 * Author: rafael.luiz.cancian
 * 
 * Created on 31 de Maio de 2019, 08:37
 */

#include <fstream>
#include <stdio.h>
#include <iostream>
#include <cstdio>

#include "ModelManager.h"
#include "List.h"
#include "Simulator.h"

//using namespace GenesysKernel;

ModelManager::ModelManager(Simulator* simulator) {
	_simulator = simulator;
	_currentModel = nullptr;
}

Model* ModelManager::newModel() {
	_currentModel = new Model(_simulator);
	return _currentModel;
}

void ModelManager::insert(Model* model) {
	_models->insert(model);
	this->_currentModel = model;
	_simulator->getTracer()->trace(Util::TraceLevel::L3_results, "Model successfully inserted");
}

void ModelManager::remove(Model* model) {
	_models->remove(model);
	if (_currentModel == model) {
		_currentModel = this->front();
	}
	model->~Model();
	_simulator->getTracer()->trace(Util::TraceLevel::L3_results, "Model successfully removed");
}

unsigned int ModelManager::size() {
	return _models->size();
}

bool ModelManager::saveModel(std::string filename) {
	if (_currentModel != nullptr)
		return _currentModel->save(filename);
	return false;
}

bool ModelManager::loadModel(std::string filename) {
	Model* model = new Model(_simulator);
	bool res = model->load(filename);
	if (res) {
		this->insert(model);
		_simulator->getTracer()->trace(Util::TraceLevel::L3_results, "Model successfully loaded");
	} else {
		model->~Model();
		_simulator->getTracer()->trace(Util::TraceLevel::L3_results, "Model coud not be loaded");
	}
	return res;
}

bool ModelManager::createFromLanguage(std::string modelSpecification) {
	std::string randomTempFilename = "_tmp";
	const char alphanum[] =
			"0123456789_."
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz";
	unsigned short stringLength = sizeof (alphanum) - 1;
	for (unsigned short i = 0; i < 16; i++) {
		randomTempFilename += alphanum[rand() % stringLength];
	}
	std::ofstream savefile;
	savefile.open("tmp_createfromlanguage.tmp" /*randomTempFilename*/, std::ofstream::out);
	savefile << modelSpecification;
	savefile.close();
	bool result = this->loadModel(randomTempFilename);
	//const std::string filename = randomTempFilename;
	std::remove("tmp_createfromlanguage.tmp" /*randomTempFilename*/);
	return result;
}

void ModelManager::setCurrent(Model* model) {
	this->_currentModel = model;
}

Model* ModelManager::current() {
	return _currentModel;
}

Model* ModelManager::front() {
	return _models->front();
}

Model* ModelManager::next() {
	return _models->next();
}
