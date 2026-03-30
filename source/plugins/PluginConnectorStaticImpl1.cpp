#include "PluginConnectorStaticImpl1.h"

Plugin* PluginConnectorStaticImpl1::check(const std::string dynamicLibraryFilename) {
    (void) dynamicLibraryFilename;
    return nullptr;
}

Plugin* PluginConnectorStaticImpl1::connect(const std::string dynamicLibraryFilename) {
    (void) dynamicLibraryFilename;
    return nullptr;
}

bool PluginConnectorStaticImpl1::disconnect(const std::string dynamicLibraryFilename) {
    (void) dynamicLibraryFilename;
    return true;
}

bool PluginConnectorStaticImpl1::disconnect(Plugin* plugin) {
    (void) plugin;
    return true;
}
