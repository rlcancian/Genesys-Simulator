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

#include "kernel/util/List.h"
#include <string>
#include "kernel/simulator/PluginConnector_if.h"
//namespace GenesysKernel {

/*!
 * \brief In-memory plugin connector that resolves built-in plugin metadata.
 *
 * This connector does not load shared libraries. Instead, it maps known file
 * names to compiled-in plugin classes and returns the corresponding metadata.
 */
class PluginConnectorDummyImpl1 : public PluginConnector_if {
public:
	PluginConnectorDummyImpl1();
	virtual ~PluginConnectorDummyImpl1() = default;
public:
	List<Plugin*>* check(const std::string dynamicLibraryFilename) override;
	List<Plugin*>* connect(const std::string dynamicLibraryFilename) override;
	List<std::string>* find() override;
	bool disconnect(const std::string dynamicLibraryFilename) override;
	bool disconnect(Plugin* plugin) override;

private:
	// Looks up the compiled-in GetPluginInformation for a given file alias.
	// Returns nullptr if the alias isn't recognized.
	Plugin* _resolve(const std::string& dynamicLibraryFilename) const;
};

//namespace\\}
#endif /* PLUGINCONNECTORDUMMYIMPL1_H */
