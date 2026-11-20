/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   PickUp.cpp
 * Author: rlcancian
 * 
 * Created on 03 de Junho de 2019, 15:15
 */

#include "plugins/components/Decisions/PickUp.h"
#include "kernel/simulator/model/Model.h"
#include <exception>

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &PickUp::GetPluginInformation;
}
#endif

ModelDataDefinition* PickUp::NewInstance(Model* model, std::string name) {
	return new PickUp(model, name);
}

ModelComponent* PickUp::LoadInstance(Model* model, PersistenceRecord* fields) {
	auto* instance = new PickUp(model);
	try {
		instance->_loadInstance(fields);
	} catch (const std::exception& e) {
		instance->traceError("Failed to load PickUp instance: " + std::string(e.what()));
	}
	return instance;
}

PickUp::PickUp(Model* model, std::string name) : Remove(model, std::move(name), Util::TypeOf<PickUp>()) {
}

PluginInformation* PickUp::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<PickUp>(), &PickUp::LoadInstance, &PickUp::NewInstance);
	info->setCategory("Decisions");
	info->insertDynamicLibFileDependence("remove.so");
	info->setMinimumOutputs(2);
	info->setMaximumOutputs(2);
	// ...
	return info;
}
