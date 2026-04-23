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
#include "../util/List.h"
#include "Simulator.h"

//using namespace GenesysKernel;

namespace {
void traceModelManager(Simulator* simulator, const std::string& message) {
	if (simulator != nullptr && simulator->getTraceManager() != nullptr) {
		simulator->getTraceManager()->trace(TraceManager::Level::L2_results, message);
	}
}
}

ModelManager::ModelManager(Simulator* simulator) {
	_simulator = simulator;
}

ModelManager::~ModelManager() {
	if (_models != nullptr) {
		for (Model* model : *_models->list()) {
			delete model;
		}
		delete _models;
		_models = nullptr;
	}
	_currentModel = nullptr;
}

Model* ModelManager::newModel() {
	Model* model = new Model(_simulator);
	insert(model);
	return model;
}

void ModelManager::insert(Model* model) {
	if (model == nullptr) {
		return;
	}
	if (hasModel(model)) {
		_currentModel = model;
		return;
	}
	_models->insert(model);
	this->_currentModel = model;
	traceModelManager(_simulator, "Model successfully inserted");
}

void ModelManager::remove(Model* model) {
	if (model == nullptr || !hasModel(model)) {
		return;
	}
	const int removedIndex = indexOf(model);
	_models->remove(model);
	if (_currentModel == model) {
		//Util::ResetAllIds(); // @ToDo: (importante): Util::ResetAllIds() should be MODEL BASED!!!
		if (_models->empty()) {
			_currentModel = nullptr;
		} else if (removedIndex >= 0 && static_cast<unsigned int>(removedIndex) < _models->size()) {
			_currentModel = modelAt(static_cast<unsigned int>(removedIndex));
		} else {
			_currentModel = modelAt(_models->size() - 1);
		}
	}
	delete model; //->~Model();
	traceModelManager(_simulator, "Model successfully removed");
}

unsigned int ModelManager::size() {
	return _models->size();
}

bool ModelManager::saveModel(std::string filename) {
	if (_currentModel != nullptr)
		return _currentModel->save(filename);
	return false;
}

Model* ModelManager::loadModel(std::string filename) {
	Model* model = new Model(_simulator);
	bool res = model->load(filename);
	if (res) {
		this->insert(model);
		traceModelManager(_simulator, "Model successfully loaded");
		return model;
	} else {
		delete model; //->~Model();
		traceModelManager(_simulator, "Model coud not be loaded");
		return nullptr;
	}
}

Model* ModelManager::createFromLanguage(std::string modelSpecification) {
	std::string randomTempFilename = "_tmp";
	const char alphanum[] =
			"0123456789_."
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz";
	unsigned short stringLength = sizeof (alphanum) - 1;
	for (unsigned short i = 0; i < 16; i++) {
		randomTempFilename += alphanum[rand() % stringLength];
	}
	randomTempFilename = "__TEMPFILEMODELO.GEN"; // @ToDo: (pequena alteração): Remove this line
	std::ofstream savefile;
	savefile.open(randomTempFilename, std::ofstream::out);
	savefile << modelSpecification;
	savefile.close();
	Model* model = this->loadModel(randomTempFilename);
	const char* fname = randomTempFilename.c_str();
	std::remove(fname);
	return model;
}

bool ModelManager::setCurrent(Model* model) {
	if (model == nullptr) {
		_currentModel = nullptr;
		return true;
	}
	if (!hasModel(model)) {
		return false;
	}
	_currentModel = model;
	return true;
}

Model* ModelManager::current() {
	return _currentModel;
}

std::vector<Model*> ModelManager::models() const {
	std::vector<Model*> result;
	if (_models == nullptr) {
		return result;
	}
	for (Model* model : *_models->list()) {
		result.push_back(model);
	}
	return result;
}

bool ModelManager::hasModel(Model* model) const {
	return model != nullptr && _models != nullptr && _models->find(model) != _models->list()->end();
}

int ModelManager::indexOf(Model* model) const {
	if (model == nullptr || _models == nullptr) {
		return -1;
	}
	int index = 0;
	for (Model* candidate : *_models->list()) {
		if (candidate == model) {
			return index;
		}
		++index;
	}
	return -1;
}

Model* ModelManager::modelAt(unsigned int index) const {
	if (_models == nullptr || index >= _models->size()) {
		return nullptr;
	}
	return _models->getAtRank(index);
}

int ModelManager::currentIndex() const {
	return indexOf(_currentModel);
}

Model* ModelManager::front() {
	_currentModel = _models->front();
	return _currentModel;
}

Model* ModelManager::last() {
	_currentModel = _models->last();
	return _currentModel;
}

Model* ModelManager::next() {
	if (!canGoNext()) {
		return nullptr;
	}
	_currentModel = modelAt(static_cast<unsigned int>(currentIndex() + 1));
	return _currentModel;
}

Model* ModelManager::previous() {
	if (!canGoPrevious()) {
		return nullptr;
	}
	_currentModel = modelAt(static_cast<unsigned int>(currentIndex() - 1));
	return _currentModel;
}

bool ModelManager::canGoNext() const {
	const int index = currentIndex();
	return index >= 0 && static_cast<unsigned int>(index + 1) < _models->size();
}

bool ModelManager::canGoPrevious() const {
	return currentIndex() > 0;
}
