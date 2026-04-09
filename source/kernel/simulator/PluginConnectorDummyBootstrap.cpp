#include "PluginConnectorDummyImpl1.h"

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

StaticGetPluginInformation PluginConnectorDummyImpl1::_connectBasic(const std::string) {
    return nullptr;
}

StaticGetPluginInformation PluginConnectorDummyImpl1::_connectContinuos(const std::string) {
    return nullptr;
}

StaticGetPluginInformation PluginConnectorDummyImpl1::_connectDiscrete(const std::string) {
    return nullptr;
}

StaticGetPluginInformation PluginConnectorDummyImpl1::_connectInputOutput(const std::string) {
    return nullptr;
}

StaticGetPluginInformation PluginConnectorDummyImpl1::_connectIntegrations(const std::string) {
    return nullptr;
}

StaticGetPluginInformation PluginConnectorDummyImpl1::_connectNetwork(const std::string) {
    return nullptr;
}

StaticGetPluginInformation PluginConnectorDummyImpl1::_connectTransfer(const std::string) {
    return nullptr;
}

StaticGetPluginInformation PluginConnectorDummyImpl1::_connectElectronicDomain(const std::string) {
    return nullptr;
}

StaticGetPluginInformation PluginConnectorDummyImpl1::_connectBiochemicalDomain(const std::string) {
    return nullptr;
}
