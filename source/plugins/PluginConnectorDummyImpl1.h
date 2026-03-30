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
//namespace GenesysKernel {

class PluginConnectorDummyImpl1 : public PluginConnector_if {
public:
	PluginConnectorDummyImpl1();
	virtual ~PluginConnectorDummyImpl1() = default;
public:
	virtual Plugin* check(const std::string dynamicLibraryFilename);
	virtual Plugin* connect(const std::string dynamicLibraryFilename);
	virtual bool disconnect(const std::string dynamicLibraryFilename);
	virtual bool disconnect(Plugin* plugin);
private:
    //StaticGetPluginInformation _connectModelDefinitions(const std::string fn);
    StaticGetPluginInformation _connectBasic(const std::string fn);
    StaticGetPluginInformation _connectContinuos(const std::string fn);
    StaticGetPluginInformation _connectDiscrete(const std::string fn);
    StaticGetPluginInformation _connectInputOutput(const std::string fn);
    StaticGetPluginInformation _connectIntegrations(const std::string fn);
    StaticGetPluginInformation _connectNetwork(const std::string fn);
    StaticGetPluginInformation _connectTransfer(const std::string fn);
    StaticGetPluginInformation _connectElectronicDomain(const std::string fn);
    StaticGetPluginInformation _connectBiochemicalDomain(const std::string fn);
};
//namespace\\}
#endif /* PLUGINCONNECTORDUMMYIMPL1_H */

