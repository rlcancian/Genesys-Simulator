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

PluginConnectorDynamicLibraryLoader::PluginConnectorDynamicLibraryLoader() {
  const char *xdg = getenv("XDG_DATA_HOME");
  _localPluginDir =
      xdg ? std::string(xdg) + "/genesys/plugins"
          : std::string(getenv("HOME")) + "/.local/lib/genesys/plugins";
}

List<Plugin*>* PluginConnectorDynamicLibraryLoader::check(
    const std::string dynamicLibraryFilename) {
  void *handle;
  try {
    handle = StaticCppCompiler::loadLibrary(dynamicLibraryFilename.c_str());
  } catch (StaticCppCompilerError &err) {
    throw PluginConnectorDynamicLibraryLoaderError(
        ("Unable to check plugin: " + std::string(err.what())).c_str());
  }

  auto *plugins = new List<Plugin*>();
  for (int i = 0;; ++i) {
    std::string symbol = "GetPluginInformation_" + std::to_string(i);
    dlerror();
    auto fn = reinterpret_cast<StaticGetPluginInformation>(
        dlsym(handle, symbol.c_str()));
    if (!fn) break;
    plugins->insert(new Plugin(fn));
  }
  dlclose(handle); 
  return plugins;
}

List<Plugin*>* PluginConnectorDynamicLibraryLoader::connect(
    const std::string dynamicLibraryFilename) {
  void *handle;
  try {
    handle = StaticCppCompiler::loadLibrary(dynamicLibraryFilename.c_str());
  } catch (StaticCppCompilerError &err) {
    throw PluginConnectorDynamicLibraryLoaderError(
        ("Unable to connect to plugin: " + std::string(err.what())).c_str());
  }

  auto *plugins = new List<Plugin*>();
  for (int i = 0;; ++i) {
    std::string symbol = "GetPluginInformation_" + std::to_string(i);
    dlerror();
    auto fn = reinterpret_cast<StaticGetPluginInformation>(
        dlsym(handle, symbol.c_str()));
    if (!fn) break;

    PluginInformation *info = fn();
    std::string typeName = info->getPluginTypename();
    if (_pluginRegistry.find(typeName) != _pluginRegistry.end())
      continue;

    _pluginRegistry[typeName] = handle;
    plugins->insert(new Plugin(fn));
  }
  return plugins;
}

List<std::string>* PluginConnectorDynamicLibraryLoader::find() {
  namespace fs = std::filesystem;
  std::vector<std::string> names;
  auto scan = [&](const fs::path &dir) {
    if (!fs::exists(dir)) return;
    for (auto &entry : fs::directory_iterator(dir))
      if (entry.path().extension() == ".so") {
        std::string p = entry.path().string();
        if (std::find(names.begin(), names.end(), p) == names.end())
          names.push_back(p);
      }
  };
  scan(_globalPluginDir);
  scan(_localPluginDir);

  auto *result = new List<std::string>();
  for (auto &n : names) result->insert(n);
  return result;
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

const char *PluginConnectorDynamicLibraryLoaderError::what() const noexcept {
  return message.c_str();
}
