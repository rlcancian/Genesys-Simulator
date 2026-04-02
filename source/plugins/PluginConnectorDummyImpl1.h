/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   PluginConnectorDummyImpl1.h
 * Author: rlcancian
 *
 * Created on 9 de Setembro de 2019, 19:24
 */

#ifndef PLUGINCONNECTORDUMMYIMPL1_H
#define PLUGINCONNECTORDUMMYIMPL1_H

#include "../kernel/simulator/PluginConnector_if.h"
#include "kernel/util/List.h"
//namespace GenesysKernel {

class PluginConnectorDummyImpl1 : public PluginConnector_if {
public:
	PluginConnectorDummyImpl1();
	virtual ~PluginConnectorDummyImpl1() = default;
public:
	Plugin* check(const std::string dynamicLibraryFilename) override;
	Plugin* connect(const std::string dynamicLibraryFilename) override;
	List<std::string>* find() override;
	bool disconnect(const std::string dynamicLibraryFilename) override;
	bool disconnect(Plugin* plugin) override;
private:
	void _connect(List<Plugin*>* plugins, Plugin* plugin);
};
//namespace\\}
#endif /* PLUGINCONNECTORDUMMYIMPL1_H */
