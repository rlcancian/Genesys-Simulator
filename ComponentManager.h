/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ComponentManager.h
 * Author: rafael.luiz.cancian
 *
 * Created on 28 de Maio de 2019, 10:41
 */

#ifndef COMPONENTMANAGER_H
#define COMPONENTMANAGER_H

#include "ModelComponent.h"

//namespace GenesysKernel {
	//class Model;

	class ComponentManager {
	public:
		ComponentManager(Model* model);
		virtual ~ComponentManager() = default;
	public:
		bool insert(ModelComponent* comp);
		void remove(ModelComponent* comp);
		ModelComponent* find(std::string name);
		void clear();
		//bool check(ModelComponent* comp, std::string expressionName, std::string* errorMessage);
		//bool check(std::string compName, std::string expressionName, bool mandatory, std::string* errorMessage);    
	public:
		//ModelComponent* getComponent(Util::identification id);
		//ModelComponent* getComponent(std::string name);
		unsigned int getNumberOfComponents();
		std::list<ModelComponent*>::iterator begin();
		std::list<ModelComponent*>::iterator end();
		ModelComponent* front();
		ModelComponent* next();
		bool hasChanged() const;
		void setHasChanged(bool _hasChanged);
		//int getRankOf(std::string name);
	public:
	private:
		List<ModelComponent*>* _components;
		Model* _parentModel;
		bool _hasChanged = false;
		std::list<ModelComponent*>::iterator _componentIterator;
	};
//namespace\\}
#endif /* COMPONENTMANAGER_H */

