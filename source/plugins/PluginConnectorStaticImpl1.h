#ifndef PLUGINCONNECTORSTATICIMPL1_H
#define PLUGINCONNECTORSTATICIMPL1_H

#include "kernel/simulator/PluginConnector_if.h"

class PluginConnectorStaticImpl1 : public PluginConnector_if {
public:
    PluginConnectorStaticImpl1() = default;
    virtual ~PluginConnectorStaticImpl1() = default;

    Plugin* check(const std::string dynamicLibraryFilename) override;
    Plugin* connect(const std::string dynamicLibraryFilename) override;
    List<std::string>* find() override;
    bool disconnect(const std::string dynamicLibraryFilename) override;
    bool disconnect(Plugin* plugin) override;
};

#endif
