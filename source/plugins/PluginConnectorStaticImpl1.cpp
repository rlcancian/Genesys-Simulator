#include "PluginConnectorStaticImpl1.h"
#include "kernel/util/List.h"

Plugin* PluginConnectorStaticImpl1::check(const std::string dynamicLibraryFilename) {
    (void) dynamicLibraryFilename;
    return nullptr;
}

Plugin* PluginConnectorStaticImpl1::connect(const std::string dynamicLibraryFilename) {
    (void) dynamicLibraryFilename;
    return nullptr;
}

List<std::string>* PluginConnectorStaticImpl1::find() {
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
