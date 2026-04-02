/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   PluginConnector_if.h
 * Author: rlcancian
 *
 * Created on 9 de Setembro de 2019, 19:17
 */

#ifndef PLUGINCONNECTOR_IF_H
#define PLUGINCONNECTOR_IF_H

#include<string>
#include "Plugin.h"
#include "../util/List.h"

//namespace GenesysKernel {

class PluginConnector_if {
public:
	/*!
	 * \brief check
	 * \param dynamicLibraryFilename
	 * \return
	 */
	/*! \brief Validates whether a dynamic library contains a compatible plugin without permanently connecting it. */
	virtual Plugin* check(const std::string dynamicLibraryFilename) = 0;
	/*!
	 * \brief connect
	 * \param dynamicLibraryFilename
	 * \return
	 */
	/*! \brief Loads and connects a plugin from a dynamic library. */
	virtual Plugin* connect(const std::string dynamicLibraryFilename) = 0;
	/*!
	 * \brief find dynamicLibraryFilenames
	 * \param
	 * \return List of dynamicLibraryFilenames
	 */
	/*! \brief Searches for candidate plugin libraries in the configured environment. */
	virtual List<std::string>* find() = 0;

	/*!
	 * \brief disconnect
	 * \param dynamicLibraryFilename
	 * \return List of all Plugins found and connected
	 */
	/*! \brief Disconnects the plugin associated with the provided library file. */
	virtual bool disconnect(const std::string dynamicLibraryFilename) = 0;
	/*!
	 * \brief disconnect
	 * \param plugin
	 * \return
	 */
	/*! \brief Disconnects a previously connected plugin instance. */
	virtual bool disconnect(Plugin* plugin) = 0;
};
//namespace\\}
#endif /* PLUGINCONNECTOR_IF_H */
