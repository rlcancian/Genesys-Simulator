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
	virtual Plugin* check(const std::string dynamicLibraryFilename) = 0;
	/*!
	 * \brief connect
	 * \param dynamicLibraryFilename
	 * \return
	 */
	virtual Plugin* connect(const std::string dynamicLibraryFilename) = 0;
	/*!
	 * \brief find dynamicLibraryFilenames
	 * \param
	 * \return List of dynamicLibraryFilenames
	 */
	virtual List<std::string>* find() = 0;

	/*!
	 * \brief disconnect
	 * \param dynamicLibraryFilename
	 * \return List of all Plugins found and connected
	 */
	virtual bool disconnect(const std::string dynamicLibraryFilename) = 0;
	/*!
	 * \brief disconnect
	 * \param plugin
	 * \return
	 */
	virtual bool disconnect(Plugin* plugin) = 0;
};
//namespace\\}
#endif /* PLUGINCONNECTOR_IF_H */
