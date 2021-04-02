/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ModelManager.h
 * Author: rafael.luiz.cancian
 *
 * Created on 31 de Maio de 2019, 08:37
 */

#ifndef MODELMANAGER_H
#define MODELMANAGER_H

#include "Model.h"
#include "TraceManager.h"

//namespace GenesysKernel {

	class ModelManager {
	public:
		ModelManager(Simulator* simulator);
		virtual ~ModelManager() = default;
	public:
		Model* newModel();
		void insert(Model* model);
		void remove(Model* model);
		void setCurrent(Model* model);
		bool saveModel(std::string filename);
		bool loadModel(std::string filename);
		unsigned int size();
	public:
		Model* front();
		Model* current();
		Model* next();
		//Model* end();
	private:
		List<Model*>* _models = new List<Model*>();
		Model* _currentModel;
	private:
		Simulator* _simulator;
	};
//namespace\\}
#endif /* MODELMANAGER_H */

