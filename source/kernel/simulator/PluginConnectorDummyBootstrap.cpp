// Use an explicit relative include so kernel runtime builds can resolve the dummy connector header.
#include "../../plugins/PluginConnectorDummyImpl1.h"

PluginConnectorDummyImpl1::PluginConnectorDummyImpl1() = default;

Plugin* PluginConnectorDummyImpl1::check(const std::string) {
    return nullptr;
}

Plugin* PluginConnectorDummyImpl1::connect(const std::string) {
    return nullptr;
}

bool PluginConnectorDummyImpl1::disconnect(const std::string) {
    return false;
}

bool PluginConnectorDummyImpl1::disconnect(Plugin*) {
    return false;
}

// Return an empty discovery list for the bootstrap stub when plugin scanning is unavailable.
List<std::string>* PluginConnectorDummyImpl1::find() {
    return new List<std::string>();
}

// Keep bootstrap connector behavior as a no-op to avoid mutating plugin registries in unit test builds.
void PluginConnectorDummyImpl1::_connect(List<Plugin*>*, Plugin*) {}
