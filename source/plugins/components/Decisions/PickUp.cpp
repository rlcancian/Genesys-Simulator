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

#ifdef PLUGINCONNECT_DYNAMIC

extern "C" StaticGetPluginInformation GetPluginInformation() {
	return &PickUp::GetPluginInformation;
}
#endif

ModelDataDefinition* PickUp::NewInstance(Model* model, std::string name) {
	return new PickUp(model, name);
}

PickUp::PickUp(Model* model, std::string name) : Remove(model, name) {
	//Util::TypeOf<PickUp>();
}

PluginInformation* PickUp::GetPluginInformation() {
	PluginInformation* info = new PluginInformation(Util::TypeOf<Remove>(), &Remove::LoadInstance, &Remove::NewInstance);
	info->setCategory("Decisions");
	info->insertDynamicLibFileDependence("remove.so");
	info->setMinimumOutputs(2);
	info->setMaximumOutputs(2);
	// ...
	return info;
}
