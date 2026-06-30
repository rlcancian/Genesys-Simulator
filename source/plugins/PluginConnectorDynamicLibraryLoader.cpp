// TODO: Make this class portable.
// Currently supports only Unix compliant systems.

#include <filesystem>
#include <string>
#include <vector>

#include <dlfcn.h>

#include "../kernel/TraitsKernel.h"
#include "../kernel/simulator/PluginInformation.h"
#include "./PluginConnectorDynamicLibraryLoader.h"
#include "StaticCppCompiler.h"

using GetPluginInformation = StaticGetPluginInformation (*)();

PluginConnectorDynamicLibraryLoader::PluginConnectorDynamicLibraryLoader() {}

// ~PluginConnectorDynamicLibraryLoader();

// TODO: Does this method even make sense?
// connect already throws exceptions the caller can handle.
Plugin *PluginConnectorDynamicLibraryLoader::check(
    const std::string dynamicLibraryFilename) {
  return connect(dynamicLibraryFilename);
}

Plugin *PluginConnectorDynamicLibraryLoader::connect(
    const std::string dynamicLibraryFilename) {
  // Try loading library from default paths.
  void *dynamicLibraryHandle;
  try {
    dynamicLibraryHandle =
        StaticCppCompiler::loadLibrary(dynamicLibraryFilename.c_str());

  } catch (StaticCppCompilerError &err) {
    auto msg = "Unable to connect to plugin: " + std::string(err.what());
    throw PluginConnectorDynamicLibraryLoaderError(msg.c_str());
  }

  // Strip file extension if the filepath includes it.
  std::string strippedFilename =
      _stripDynamicLibraryFileExtension(dynamicLibraryFilename);

  // Look for symbol in the loaded library.
  auto getPluginInformation =
      reinterpret_cast<StaticGetPluginInformation (*)()>(
          dlsym(dynamicLibraryHandle, "GetPluginInformation"));
  const char *error = dlerror();
  if (error != nullptr)
    throw PluginConnectorDynamicLibraryLoaderError(error);

  StaticGetPluginInformation staticGetPluginInformation =
      getPluginInformation();
  PluginInformation *info = staticGetPluginInformation();
  std::string pluginTypename = info->getPluginTypename();

  // Insert plugin handle into registry.
  if (_pluginRegistry.find(pluginTypename) == _pluginRegistry.end()) {
    std::string error_message =
        "Unable to connect to plugin: " + pluginTypename +
        " already in registry";
    throw PluginConnectorDynamicLibraryLoaderError(error_message.c_str());
    _pluginRegistry[pluginTypename] = dynamicLibraryHandle;
  }

  return new Plugin(staticGetPluginInformation);
}

List<std::string> *PluginConnectorDynamicLibraryLoader::find() {
  namespace fs = std::filesystem;
  auto pluginNames = std::vector<std::string>();

  auto scan = [&](const fs::path &dir) {
    if (!fs::exists(dir))
      return;
    for (const auto &entry : fs::directory_iterator(dir)) {
      if (entry.path().extension() == ".so") {
        std::string name = entry.path().filename().string();
        // avoid duplicates — local overrides global
        if (std::find(pluginNames.begin(), pluginNames.end(), name) ==
            pluginNames.end())
          pluginNames.push_back(name);
      }
    }
  };
  scan(TraitsKernel<PluginConnector_if>::globalPluginDir);

  const char *xdg = getenv("XDG_DATA_HOME");
  std::string local =
      xdg ? std::string(xdg) + "/genesys/plugins"
          : std::string(getenv("HOME")) + "/.local/lib/genesys/plugins";
  scan(local);

  auto plugins = new List<std::string>();
  for (int i = 0; i < pluginNames.size(); i++)
    plugins->insert(pluginNames[i]);

  return plugins;
}

bool PluginConnectorDynamicLibraryLoader::disconnect(
    const std::string dynamicLibraryFilename) {
  auto stripped = _stripDynamicLibraryFileExtension(dynamicLibraryFilename);
  if (_pluginRegistry.find(stripped) != _pluginRegistry.end()) {
    void *handle = _pluginRegistry[stripped];
    _pluginRegistry.erase(stripped);

    int closed = dlclose(handle);
    if (closed != 0) {
      throw PluginConnectorDynamicLibraryLoaderError(dlerror());
    }
    return true;
  }
  return false;
}

bool PluginConnectorDynamicLibraryLoader::disconnect(Plugin *plugin) {
  return true;
}

static std::filesystem::path localPluginDir() {
  const char *xdg = getenv("XDG_DATA_HOME");
  std::string base = xdg ? xdg : std::string(getenv("HOME")) + "/.local/lib";
  return base + "/genesys/plugins";
}

std::string
PluginConnectorDynamicLibraryLoader::_stripDynamicLibraryFileExtension(
    std::string filename) const {
  std::string stripped;
  if (filename.substr(filename.size() - 3, filename.size()) == ".so") {
    stripped = filename.substr(0, filename.size() - 3);
  } else if (filename.substr(filename.size() - 4, filename.size()) == ".dll") {
    stripped = filename.substr(0, filename.size() - 4);
  }
  return stripped;
}

PluginConnectorDynamicLibraryLoaderError::
    PluginConnectorDynamicLibraryLoaderError(const char *msg)
    : message(msg) {}

const char* PluginConnectorDynamicLibraryLoaderError::what() const noexcept {
  return message.c_str();
}
