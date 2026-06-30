/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   PluginConnectorDynamicLibraryLoader.h
 * Author: Jarne Demoen, Otávio Augusto S. Jatobá
 *
 * Created on June 24 2026, 14:25
 */

#pragma once

#include <exception>
#include <filesystem>
#include <map>
#include <string>

#include "../kernel/simulator/Plugin.h"
#include "../kernel/simulator/PluginConnector_if.h"
#include "../kernel/util/List.h"

class PluginConnectorDynamicLibraryLoader : public PluginConnector_if {
public:
  PluginConnectorDynamicLibraryLoader();
  virtual ~PluginConnectorDynamicLibraryLoader() = default;

  List<Plugin*>* check(const std::string dynamicLibraryFilename) override;
  /*!
   * \brief connect
   * \param dynamicLibraryFilename
   * \return
   */
  /*! \brief Loads and connects a plugin from a dynamic library. */
  List<Plugin*>* connect(const std::string dynamicLibraryFilename) override;
  /*!
   * \brief find dynamicLibraryFilenames
   * \param
   * \return List of dynamicLibraryFilenames
   */
  /*! \brief Searches for candidate plugin libraries in the configured
   * environment. */
  List<std::string> *find() override;

  /*!
   * \brief disconnect
   * \param dynamicLibraryFilename
   * \return List of all Plugins found and connected
   */
  /*! \brief Disconnects the plugin associated with the provided library file.
   */
  bool disconnect(const std::string dynamicLibraryFilename) override;
  /*!
   * \brief disconnect
   * \param plugin
   * \return
   */
  /*! \brief Disconnects a previously connected plugin instance. */
  bool disconnect(Plugin *plugin) override;

  static std::filesystem::path localPluginDir();

private:
	const std::string _globalPluginDir = "/usr/local/lib/genesys/plugins";
  std::string _localPluginDir;
  std::map<std::string, void *> _pluginRegistry;

  std::string _stripDynamicLibraryFileExtension(std::string) const;
};

class PluginConnectorDynamicLibraryLoaderError : public std::exception {
private:
  std::string message;

public:
  PluginConnectorDynamicLibraryLoaderError(const char *msg);
  ~PluginConnectorDynamicLibraryLoaderError() = default;
  const char *what() const noexcept override;
};
