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

#include "kernel/simulator/PluginConnector_if.h"
#include "kernel/util/List.h"
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
	/*!
	 * \brief Verifies whether a built-in plugin name can be resolved.
	 * \param dynamicLibraryFilename Plugin library filename or alias.
	 * \return Metadata for the resolved plugin, or \c nullptr if unknown.
	 */
	Plugin* check(const std::string dynamicLibraryFilename) override;
	/*!
	 * \brief Resolves a built-in plugin by filename alias.
	 * \param dynamicLibraryFilename Plugin library filename or alias.
	 * \return Metadata for the resolved plugin, or \c nullptr if unknown.
	 */
	Plugin* connect(const std::string dynamicLibraryFilename) override;
	/*!
	 * \brief Returns the list of built-in plugin filenames recognized by this connector.
	 * \return Newly allocated list with known plugin aliases.
	 */
	List<std::string>* find() override;
	/*!
	 * \brief Disconnects a plugin identified by filename.
	 * \param dynamicLibraryFilename Plugin library filename or alias.
	 * \return Always \c true in the current dummy implementation.
	 */
	bool disconnect(const std::string dynamicLibraryFilename) override;
	/*!
	 * \brief Disconnects a plugin instance.
	 * \param plugin Plugin instance to disconnect.
	 * \return Always \c true in the current dummy implementation.
	 */
	bool disconnect(Plugin* plugin) override;
private:
	void _connect(List<Plugin*>* plugins, Plugin* plugin);
};
//namespace\\}
#endif /* PLUGINCONNECTORDUMMYIMPL1_H */
